#include <gst/gst.h>
#include <iostream>
using namespace std;

int main(int argc, char *argv[]) {
    int width = 640, height = 480, framerate = 30, bitrateInKBPS = 512;
    //GstElement *pipeline, *source, *sink;
    GstElement *pipeline, *sink, *source_filter, *filter, *appsink, *h264parse, *encoder, *source, *video_convert;
    GstBus *bus;
    GstMessage *msg;
    GMainLoop *loop;
    GstStateChangeReturn ret;

    //initialize all elements
    gst_init(&argc, &argv);
    pipeline = gst_pipeline_new ("pipeline");
    //source = gst_element_factory_make ("autovideosrc", "source");
    source = gst_element_factory_make("v4l2src", "source");
    sink = gst_element_factory_make ("autovideosink", "sink");
    
    source_filter = gst_element_factory_make("capsfilter", "source_filter");
    filter = gst_element_factory_make("capsfilter", "encoder_filter");
    appsink = gst_element_factory_make("appsink", "appsink");
    h264parse = gst_element_factory_make("h264parse", "h264parse");

    encoder = gst_element_factory_make("x264enc", "encoder");

    //check for null objects
    if (!pipeline || !source || !sink) {
        cout << "not all elements created: pipeline["<< !pipeline<< "]" << "source["<< !source<< "]" << "sink["<< !sink << "]" << endl;
        return -1;
    }

    //set video source
    //g_object_set(G_OBJECT (source), "location", argv[1], NULL);
    g_object_set(G_OBJECT (source), "do-timestamp", TRUE, "device", "/dev/video0", NULL);
    cout << "==>Set video source." << endl;
    g_object_set(G_OBJECT (sink), "sync", FALSE, NULL);
    cout << "==>Set video sink." << endl;

    //Add
    GstCaps *query_caps_raw = gst_caps_new_simple("video/x-raw", "width", G_TYPE_INT, width, "height", G_TYPE_INT, height, NULL);
    cout << "==>Set x-raw width and height." << endl;

    video_convert = gst_element_factory_make("videoconvert", "video_convert");
    cout << "==>Set videoconvert." << endl;

    gst_caps_set_simple(query_caps_raw, "format", G_TYPE_STRING, "I420", NULL);
    cout << "==>Set format." << endl;

    g_object_set(G_OBJECT (source_filter), "caps", query_caps_raw, NULL);
    cout << "==>Set source filter." << endl;

    g_object_set(G_OBJECT (encoder), "bframes", 0, "key-int-max", 45, "bitrate", bitrateInKBPS, NULL);
    cout << "==>Set encoder." << endl;

    GstCaps *h264_caps = gst_caps_new_simple("video/x-h264", "stream-format", G_TYPE_STRING, "avc", "alignment", G_TYPE_STRING, "au", NULL);
    cout << "==>Set aac." << endl;

    gst_caps_set_simple(h264_caps, "profile", G_TYPE_STRING, "baseline", NULL);
    cout << "==>Set baseline." << endl;

    g_object_set(G_OBJECT (filter), "caps", h264_caps, NULL);
    cout << "==>Set h264_caps." << endl;

    //add all elements together
    //gst_bin_add_many (GST_BIN (pipeline), source, sink, NULL);
    //if (gst_element_link (source, sink) != TRUE) {
    gst_bin_add_many (GST_BIN (pipeline), source, video_convert, source_filter, encoder, h264parse, filter, appsink, NULL);
    if (gst_element_link_many (source, video_convert, source_filter, encoder, h264parse, filter, appsink) != TRUE) {
        cout << "Elements could not be linked." << endl;
        gst_object_unref (pipeline);
        return -1;
    }
    cout << "==>Link elements." << endl;

    //set the pipeline state to playing
    ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        cout << "Unable to set the pipeline to the playing state." << endl;
        gst_object_unref (pipeline);
        return -1;
    }
    cout << "==>Set video to play." << endl;

    //get pipeline's bus
    bus = gst_element_get_bus (pipeline);
    cout << "==>Setup bus." << endl;

    loop = g_main_loop_new(NULL, FALSE);
    cout << "==>Begin stream." << endl;
    g_main_loop_run(loop);

    g_main_loop_unref(loop);
    gst_object_unref (bus);
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (pipeline);
}
