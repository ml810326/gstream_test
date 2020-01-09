#include <gst/gst.h>
#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static gboolean
bus_call (GstBus *bus, GstMessage *msg, gpointer data)
{
  GMainLoop *loop = (GMainLoop *) data;

  switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:
      g_print ("End of stream\n");
      g_main_loop_quit (loop);
      break;

    case GST_MESSAGE_ERROR: {
      gchar  *debug;
      GError *error;

      gst_message_parse_error (msg, &error, &debug);
      g_free (debug);

      g_printerr ("Error: %s\n", error->message);
      g_error_free (error);

      g_main_loop_quit (loop);
      break;
    }
    default:
      break;
  }

  return TRUE;
}

int main (int argc, char *argv[])
{
  GMainLoop *loop;

  /* GstElement *pipeline, *source, *demuxer, *decoder, *conv, *sink; */
  GstElement *pipeline, *source, *encoder, *source_filter, *filter, *conv, *hparse, *qRTMP, *flvmux, *rtmpsink;
  GstBus *bus;
  guint bus_watch_id;

  char* STREAM_URL = "rtmp://test234-testvideo7-uswe.channel.media.azure.net:1935/live/d15526ff9dc54f11b02dd306d383c2fb/stream1";

  /* Initialisation */
  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);

  /* Create gstreamer elements */
  pipeline = gst_pipeline_new ("azure-camera");
  source = gst_element_factory_make ("v4l2src", "source");
  source_filter = gst_element_factory_make("capsfilter", "source_filter");
  encoder = gst_element_factory_make ("x264enc", "encoder");
  filter = gst_element_factory_make("capsfilter", "filter");
  conv = gst_element_factory_make ("videoconvert", "conv");
  hparse = gst_element_factory_make ("h264parse", "hparse");
  qRTMP = gst_element_factory_make ("queue", "qRTMP");
  flvmux = gst_element_factory_make ("flvmux", "mux");
  rtmpsink = gst_element_factory_make ("rtmpsink", "rsink");

  if (!pipeline || !source || !source_filter || !encoder || !filter || !conv || !hparse || !qRTMP || !flvmux || !rtmpsink) {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }

  /* Set up the pipeline */

  /* configure source */
  g_object_set (G_OBJECT (source), "do-timestamp", TRUE, "device", "/dev/video0", NULL);

  /* source filter*/
  GstCaps *query_caps_raw = gst_caps_new_simple("video/x-raw", "width", G_TYPE_INT, 640, "height", G_TYPE_INT, 480, NULL); 
  gst_caps_set_simple(query_caps_raw, "format", G_TYPE_STRING, "I420", NULL);
  g_object_set(G_OBJECT (source_filter), "caps", query_caps_raw, NULL);
  gst_caps_unref(query_caps_raw);

  /* encoder filter*/
  g_object_set(G_OBJECT (encoder), "bframes", 0, "key-int-max", 45, "bitrate", 500, NULL);
  GstCaps *h264_caps = gst_caps_new_simple("video/x-h264", "stream-format", G_TYPE_STRING, "avc", "alignment", G_TYPE_STRING, "au", NULL);
  gst_caps_set_simple(h264_caps, "profile", G_TYPE_STRING, "baseline", NULL);
  g_object_set(G_OBJECT (filter), "caps", h264_caps, NULL);
  gst_caps_unref(h264_caps);

  /* rtmpsink*/
  g_object_set (G_OBJECT (rtmpsink), "location", STREAM_URL, NULL);

  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  bus_watch_id = gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);

  /* we add all elements into the pipeline */
  gst_bin_add_many (GST_BIN (pipeline),
                    source, conv, source_filter, encoder, filter, hparse, qRTMP, flvmux, rtmpsink, NULL);

  /*gst_element_link_many (source, source_filter, encoder, filter, conv, hparse, qRTMP, flvmux, rtmpsink, NULL);*/
  if (!gst_element_link_many (source, conv, source_filter, encoder, filter, hparse, qRTMP, flvmux, rtmpsink, NULL)) {
      g_printerr("Elements A could not be linked.\n");
      gst_object_unref(pipeline);
      return 1;
  }

  /*audio setting*/
  GstElement *audiosrc, *audioresample, *AudioFilter, *audioEnc;
  audiosrc = gst_element_factory_make ("pulsesrc", "audiosrc");
  audioresample = gst_element_factory_make ("audioresample", "audioresample");
  AudioFilter = gst_element_factory_make("capsfilter", "AudioFilter");
  GstCaps *audi_caps = gst_caps_new_simple("audio/x-raw", "rate", G_TYPE_INT, 44100, NULL);
  g_object_set(G_OBJECT (AudioFilter), "caps", audi_caps, NULL);
  audioEnc = gst_element_factory_make ("voaacenc", "audioEnc");
  /*g_object_set(G_OBJECT (audioEnc), "bitrate", G_TYPE_INT, 9600, NULL);*/

  gst_bin_add_many (GST_BIN (pipeline), audiosrc, audioresample, AudioFilter, audioEnc, NULL);
  /*gst_element_link_many (audiosrc, audioresample, AudioFilter, audioEnc, flvmux, NULL);*/
  if (!gst_element_link_many (audiosrc, audioresample, AudioFilter, audioEnc, flvmux, NULL)) {
      g_printerr("Elements B could not be linked.\n");
      gst_object_unref(pipeline);
      return 1;
  }

  g_printerr("Set the pipeline to playing state\n");
  /* Set the pipeline to "playing" state*/
  gst_element_set_state (pipeline, GST_STATE_PLAYING);


  /* Iterate */
  g_print ("Running...\n");
  g_main_loop_run (loop);


  /* Out of the main loop, clean up nicely */
  g_print ("Returned, stopping playback\n");
  gst_element_set_state (pipeline, GST_STATE_NULL);

  g_print ("Deleting pipeline\n");
  gst_object_unref (GST_OBJECT (pipeline));
  g_source_remove (bus_watch_id);
  g_main_loop_unref (loop);

  return 0;
}
