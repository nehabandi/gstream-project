#include <gst/gst.h>
#include <string>
#include<iostream>
using namespace std;


void usbset();//function to create thumbnail for usb video
static void internal_storage_set();//function to create thumbnail for internal storage video
static int set_pipeline(string video_path, string thumbnail_path);//function to set the pipeline and do operation
static void pad_added_handler(GstElement *src, GstPad *pad, GstElement *sink);//function to add the get the pad and connect
void message_bus_handler(GstMessage *msg);// print the message from bus

int main(int argc, char *argv[]) {

    int option;
     while(1)
    {
    	 cout<<"\n Enter the storage type to generate thumbnail \n ";
    	 cout<<"\n 1--FROM USB \n";
    	 cout<<"\n 2--FROM INTERNAL STORAGE \n";
    	 cout<<"\n 3--EXIT\n";
    	 cin>>option;
    	 switch(option)
    	 {

    	 	 	case 1: //create thumbnail for video file from internal storage
    	 	 			usbset();
    	 	 			break;

    	 	 	case 2: //create thumbnail for video file from USB
    	 	 		   internal_storage_set();

    	 	 			break;
    	 	 	case 3://exit
    	 	 			exit(1);
    	 	 			break;
    	 	 	default://valid option
    	 	 			cout<<"\n ENTER THE VALID OPTION \n";
    	 	 			break;

    }
    }
    return 0;
}


void usbset()
{
    const gchar *path = "/media/ee212466/";
    GDir *dir;
    const gchar *filename;

    dir = g_dir_open(path, 0, NULL);
    if (dir == NULL)
    {
        g_printerr("UNABLE TO OPEN Directory\n"); // Print no directory"
        return ;
    }

    gboolean found_files = FALSE; // Add a flag to track if any files were found

    while ((filename = g_dir_read_name(dir)) != NULL) {
        gchar *video_path = g_strdup_printf("%s%s/", path, filename);
        gchar *thumbnail_path = g_strdup(video_path);
        set_pipeline(video_path, thumbnail_path);
        g_free(video_path);
        g_free(thumbnail_path);
        found_files = TRUE; // Set flag to true if any files are found
    }


    if (!found_files)
    	g_printerr("NO USB CONNECTED TO THIS PC \n"); // If no files are found, print "no usb"
    g_dir_close(dir);
}


static void internal_storage_set()
{
	//set the location of video and dest
    string video_path = "/home/ee212466/Downloads/neeha/";
    string thumbnail_path = "/home/ee212466/Downloads/neeha/";
    set_pipeline(video_path,thumbnail_path);

}

static void pad_added_handler(GstElement *src, GstPad *pad, GstElement *sink)
{
	  GstCaps *new_pad_caps = NULL;
	  GstStructure *new_pad_struct = NULL;
	  const gchar *new_pad_type = NULL;
	  GstPadLinkReturn ret;
	  GstPad *sinkpad=gst_element_get_static_pad(sink, "sink");
	  g_print ("Received new pad '%s' from '%s':\n", GST_PAD_NAME (pad), GST_ELEMENT_NAME (src));
    /* Check the new pad's type */
	  new_pad_caps = gst_pad_get_current_caps (pad);
	  new_pad_struct = gst_caps_get_structure (new_pad_caps, 0);
	  new_pad_type = gst_structure_get_name (new_pad_struct);
	 if (g_str_has_prefix (new_pad_type, "video"))
	 {
		 g_print ("It has type '%s'\n", new_pad_type);

		 ret = gst_pad_link (pad, sinkpad);
		 if (GST_PAD_LINK_FAILED (ret))
			  g_print ("Type is '%s' but link failed.\n", new_pad_type);
		 else
		 	 	g_print ("Link succeeded (type '%s').\n", new_pad_type);
	 }
	 if (new_pad_caps != NULL)
	     gst_caps_unref (new_pad_caps);
	  gst_object_unref(sinkpad);
}



static int set_pipeline(string video_path, string thumbnail_path)
{
    gst_init(NULL, NULL);
    GError *error = NULL;

    GDir *dir = g_dir_open(video_path.c_str(), 0, &error);
    if (dir == NULL)
    {
        g_printerr("Failed to open video path  directory no thumbnails can be generated: %s\n", error->message);
        g_error_free(error);
       return 1;
    }

    const gchar *filename = NULL;
    int index = 1;
    bool found_video = false;

    while ((filename = g_dir_read_name(dir)) != NULL)
    {
        if ((g_str_has_suffix(filename, ".mp4")) || (g_str_has_suffix(filename, ".avi")) || (g_str_has_suffix(filename, ".webm")))
        {
            found_video = true;
            string input_path = video_path + filename;

            GDir *dir1 = g_dir_open(thumbnail_path.c_str(), 0, &error);
    if (dir1 == NULL)
    {
        g_printerr("Failed to open thumbnail directory: %s\n", error->message);
        g_error_free(error);
       return 1;
    }

            	string output_path = thumbnail_path + g_path_get_basename(filename) + "_output.jpg";
            	cout << index++ << ")  " << input_path << endl << endl;

            	//create all the plugin used for this project
            	GstElement *pipeline = gst_pipeline_new(NULL);
    			GstElement *src = gst_element_factory_make("filesrc", "src");
    			GstElement *decode = gst_element_factory_make("decodebin", "decode");
    			GstElement *convert = gst_element_factory_make("videoconvert", "convert");
    			GstElement *rate = gst_element_factory_make("videorate", "rate");
    			GstElement *scale = gst_element_factory_make("videoscale", "scale");
    			GstElement *capsfilter = gst_element_factory_make("capsfilter", "capsfilter");
    			GstElement *encode = gst_element_factory_make("jpegenc", "encode");
    			GstElement *sink = gst_element_factory_make("filesink", "sink");

    			//check whether the element created sucessfully
    			if (!pipeline || !src || !decode || !convert || !rate || !scale || !capsfilter || !encode || !sink)
    			{
    				g_printerr("Failed to create elements\n");
    				return 1;

    			}
    			//add all elements into pipeline
    			gst_bin_add_many(GST_BIN(pipeline), src, decode, convert, rate, scale, capsfilter, encode, sink, NULL);

    			//set the video location
    			g_object_set(G_OBJECT(src), "location", input_path.c_str(), NULL);

    			//set the destination location
    			g_object_set(G_OBJECT(sink), "location", output_path.c_str(), NULL);

    			//setting the frame rate , width and height of output file ie thumbnail.jpg
    			GstCaps *caps = gst_caps_new_simple("video/x-raw","framerate", GST_TYPE_FRACTION, 1, 100,"width", G_TYPE_INT, 320,"height", G_TYPE_INT, 240,NULL);
    			g_object_set(G_OBJECT(capsfilter), "caps", caps, NULL);

    			//connect the pad added signal here we only need video pad
    			g_signal_connect(decode,"pad-added",G_CALLBACK(pad_added_handler),convert);

    			//check whether linking is done or not
    			if (!gst_element_link(src, decode) || !gst_element_link_many(convert, rate, scale, capsfilter, encode, sink, NULL))
    			{
    				g_printerr("Failed to link elements\n");
    				return 1;

    			}

    			/*start playing pipeline*/
    			GstStateChangeReturn ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
    			if (ret == GST_STATE_CHANGE_FAILURE)
    			{
    				g_printerr("Failed to start pipeline\n");
    				gst_object_unref(pipeline);
    			}

    			/* Wait until error or EOS */
    			GstBus *bus = gst_element_get_bus(pipeline);
    			GstMessage *msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GstMessageType(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));

    			if (msg != NULL)
    				message_bus_handler(msg);


    			/*pause and stop the pipeline */
    			gst_element_set_state(pipeline, GST_STATE_PAUSED);
    			gst_element_set_state(pipeline, GST_STATE_NULL);

    			/* Free resources */
    			gst_object_unref(bus);
    			gst_object_unref(pipeline);
    			gst_caps_unref(caps);
    		}
        }

    //print error msg if no video file in directory
    if (!found_video)
    {
        g_printerr("No video files found in the directory.\n");
    }

    g_dir_close(dir);
    return 0;
}


void message_bus_handler(GstMessage *msg)
{

	GError *err = NULL;
	gchar *debug_info = NULL;
	switch (GST_MESSAGE_TYPE(msg))
	{
		case GST_MESSAGE_ERROR:
			 	 	 	 	   gst_message_parse_error (msg, &err, &debug_info);
			 	 	 	 	   g_printerr ("Error received from element %s: %s\n", GST_OBJECT_NAME (msg->src), err->message);
			 	 	 	 	   g_printerr ("Debugging information: %s\n", debug_info ? debug_info : "none");
			 	 	 	 	   g_clear_error (&err);
			 	 	 	 	   g_free (debug_info);
			 	 	 	 	   break;

		case GST_MESSAGE_EOS:
						      g_print("\nEnd of pipeline \n");
							  g_print("\nTHUMBNAIL CREATED SUCESSFULLY \n");
							  g_print("\n--------------------------------------------------\n\n");
							  break;
         			default:
						      g_printerr("Unexpected message received\n");
							  break;
	}
	gst_message_unref(msg);

}
