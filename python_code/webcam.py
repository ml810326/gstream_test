#!/usr/bin/env python

import sys, os
import gi
gi.require_version('Gst', '1.0')
from gi.repository import Gst, GObject, Gtk

# command = 'v4l2src do-timestamp=TRUE device=/dev/video0 ! videoconvert ! "video/x-raw,format=I420,width=640,height=480,framerate=30/1" ! x264enc bframes=0 key-int-max=45 bitrate=500 ! "video/x-h264,stream-format=avc,alighnment=au,profile=baseline" ! h264parse ! queue ! flvmux name=mux pulsesrc ! audioresample ! audio/x-raw,rate=44100 ! queue ! voaacenc bitrate=32000 ! queue ! mux. mux. ! rtmpsink location="rtmp://test123-testvideo7-uswe.channel.media.azure.net:1935/live/d54380078c234a61888f60290cf7a047/stream1 live=1"'

class GTK_Main:
    def __init__(self):
        window = Gtk.Window(Gtk.WindowType.TOPLEVEL)
        window.set_title("Webcam-Viewer")
        window.set_default_size(500, 400)
        window.connect("destroy", Gtk.main_quit, "WM destroy")
        vbox = Gtk.VBox()
        window.add(vbox)
        self.movie_window = Gtk.DrawingArea()
        vbox.add(self.movie_window)
        hbox = Gtk.HBox()
        vbox.pack_start(hbox, False, False, 0)
        hbox.set_border_width(10)
        hbox.pack_start(Gtk.Label(), False, False, 0)
        self.button = Gtk.Button("Start")
        self.button.connect("clicked", self.start_stop)
        hbox.pack_start(self.button, False, False, 0)
        self.button2 = Gtk.Button("Quit")
        self.button2.connect("clicked", self.exit)
        hbox.pack_start(self.button2, False, False, 0)
        self.button3 = Gtk.Button("Conn")
        self.button3.connect("clicked", self.connection)
        hbox.pack_start(self.button3, False, False, 0)
        hbox.add(Gtk.Label())
        window.show_all()

        command = "v4l2src do-timestamp=TRUE device=/dev/video0 ! videoconvert ! video/x-raw,format=I420,width=640,height=480,framerate=30/1 ! x264enc bframes=0 key-int-max=45 bitrate=500 ! video/x-h264,stream-format=avc,alighnment=au,profile=baseline ! h264parse ! queue ! flvmux name=mux pulsesrc ! audioresample ! audio/x-raw,rate=44100 ! queue ! voaacenc bitrate=32000 ! autovideosink"

        # Set up the gstreamer pipeline
        # self.player = Gst.parse_launch ("v4l2src ! autovideosink")
        # self.player = Gst.parse_launch ("v4l2src do-timestamp=TRUE device=/dev/video0 ! videoconvert ! video/x-raw,format=I420,width=640,height=480,framerate=30/1 ! autovideosink")
        self.player = Gst.parse_launch (command)
        bus = self.player.get_bus()
        bus.add_signal_watch()
        bus.enable_sync_message_emission()
        bus.connect("message", self.on_message)
        bus.connect("sync-message::element", self.on_sync_message)

    def start_stop(self, w):
        if self.button.get_label() == "Start":
            self.button.set_label("Stop")
            self.player.set_state(Gst.State.PLAYING)
        else:
            self.player.set_state(Gst.State.NULL)
            self.button.set_label("Start")

    def connection(self, w):
        if self.button3.get_label() == "Conn":
            self.button3.set_label("Diss")
            pipe = Gst.parse_launch (command)
            pipe.set_state(Gst.State.PLAYING)
        else:
            pipe.set_state(Gst.State.NULL)
            self.button3.set_label("Conn")

    def exit(self, widget, data=None):
        Gtk.main_quit()

    def on_message(self, bus, message):
        t = message.type
        if t == Gst.MessageType.EOS:
            self.player.set_state(Gst.State.NULL)
            self.button.set_label("Start")
        elif t == Gst.MessageType.ERROR:
            err, debug = message.parse_error()
            print "Error: %s" % err, debug
            self.player.set_state(Gst.State.NULL)
            self.button.set_label("Start")

    def on_sync_message(self, bus, message):
        struct = message.get_structure()
        if not struct:
            return
        message_name = struct.get_name()
        if message_name == "prepare-xwindow-id":
            # Assign the viewport
            imagesink = message.src
            imagesink.set_property("force-aspect-ratio", True)
            imagesink.set_xwindow_id(self.movie_window.window.xid)

Gst.init(None)
GTK_Main()
GObject.threads_init()
Gtk.main()
