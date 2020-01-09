import gi
import time
from gi.repository import GObject, Gst
import os
gi.require_version('Gst', '1.0')
#os.environ["GST_DEBUG"] = "4"

# command = 'v4l2src do-timestamp=TRUE device=/dev/video0 ! videoconvert ! "video/x-raw,format=I420,width=640,height=480,framerate=30/1" ! x264enc bframes=0 key-int-max=45 bitrate=500 ! "video/x-h264,stream-format=avc,alighnment=au,profile=baseline" ! h264parse ! queue ! flvmux name=mux pulsesrc ! audioresample ! audio/x-raw,rate=44100 ! queue ! voaacenc bitrate=32000 ! queue ! mux. mux. ! rtmpsink location="rtmp://test123-testvideo7-uswe.channel.media.azure.net:1935/live/d54380078c234a61888f60290cf7a047/stream1 live=1"'

STREAM_URL = "rtmp://test345-testvideo10-uswe.channel.media.azure.net:1935/live/5e4af87a5301461f891566b32584c97b/stream1"

Gst.init(None)

pipeline = Gst.Pipeline()

camsrc = Gst.ElementFactory.make("v4l2src", "camsrc")
camsrc.set_property('do-timestamp', 'TRUE')
camsrc.set_property('device', '/dev/video0')
pipeline.add(camsrc)

videoconvert = Gst.ElementFactory.make("videoconvert")
pipeline.add(videoconvert)

# ccaps = Gst.Caps.from_string('video/x-raw,width=640,height=480,framerate=30/1')
ccaps = Gst.caps_from_string('video/x-raw,width=640,height=480,framerate=30/1')
CamCapsFilt = Gst.ElementFactory.make("capsfilter", "CamCapsFilt")
CamCapsFilt.set_property("caps", ccaps)
pipeline.add(CamCapsFilt)

videoEncoder = Gst.ElementFactory.make("x264enc")
videoEncoder.set_property('bframes', 0)
videoEncoder.set_property('key-int-max', 45)
videoEncoder.set_property('bitrate', 500)
vcaps = Gst.Caps.from_string('video/x-h264,stream-format=avc,alighnment=au,profile=baseline')
vCapsFilt = Gst.ElementFactory.make("capsfilter", "vCapsFilt")
vCapsFilt.set_property("caps", vcaps)
pipeline.add(videoEncoder)
pipeline.add(vCapsFilt)

h264parse = Gst.ElementFactory.make("h264parse", "h264")
pipeline.add(h264parse)

queueRTMP = Gst.ElementFactory.make("queue")
pipeline.add(queueRTMP)

flvmux = Gst.ElementFactory.make("flvmux", "mux")
pipeline.add(flvmux)

rtmpsink = Gst.ElementFactory.make("rtmpsink", "rsink")
rtmpsink.set_property("location", STREAM_URL)
pipeline.add(rtmpsink)

camsrc.link(videoconvert)
videoconvert.link(CamCapsFilt)
CamCapsFilt.link(videoEncoder)
videoEncoder.link(vCapsFilt)
vCapsFilt.link(h264parse)
h264parse.link(queueRTMP)
queueRTMP.link(flvmux)
flvmux.link(rtmpsink)

audiosrc = Gst.ElementFactory.make("pulsesrc")
pipeline.add(audiosrc)

audioresample = Gst.ElementFactory.make("audioresample")
pipeline.add(audioresample)

setAudio = Gst.Caps.from_string("audio/x-raw,rate=44100")
setAudioFilter = Gst.ElementFactory.make("capsfilter", "setAudioFilter")
setAudioFilter.set_property("caps", setAudio)
pipeline.add(setAudioFilter)

audioEncoder = Gst.ElementFactory.make("voaacenc")
audioEncoder.set_property('bitrate', 96000)
pipeline.add(audioEncoder)

audiosrc.link(audioresample)
audioresample.link(setAudioFilter)
setAudioFilter.link(audioEncoder)
audioEncoder.link(flvmux)

pipeline.set_state(Gst.State.PLAYING)
time.sleep(60)
pipeline.set_state(Gst.State.NULL)
