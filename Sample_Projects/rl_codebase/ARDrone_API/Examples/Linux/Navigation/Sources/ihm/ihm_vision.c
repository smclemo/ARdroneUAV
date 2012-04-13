/*
 * @ihm_vision.c
 * @author marc-olivier.dzeukou@parrot.com
 * @date 2007/07/27
 *
 * ihm vision source file
 *
 */

#include   <pthread.h>
#include   <gtk/gtk.h>

#include <ardrone_api.h>
#ifdef PC_USE_VISION
#    include <Vision/vision_tracker_engine.h>
#endif
#include "UI/ui.h"
#include "ihm/ihm_vision.h"
#include "ihm/ihm.h"
#include "common/mobile_config.h"
#include "ihm/ihm_stages_o_gtk.h"

#include <VP_Os/vp_os_print.h>
#include <VP_Os/vp_os_malloc.h>

extern GtkWidget *button_show_image;
extern mobile_config_t *pcfg;
/* Vision image var */
GtkLabel *label_vision_values;
GtkWidget *ihm_ImageWin, *ihm_ImageEntry[10], *ihm_Image_VBox;
int tab_vision_config_params[10];
int vision_config_options;
int image_vision_window_status, image_vision_window_view;
char label_vision_state_value[32];
extern GtkImage *image;

static void ihm_sendVisionConfigParams(GtkWidget *widget, gpointer data)
{
  api_vision_tracker_params_t params;

  params.coarse_scale       = tab_vision_config_params[0]; // scale of current picture with respect to original picture
  params.nb_pair            = tab_vision_config_params[1]; // number of searched pairs in each direction
  params.loss_per           = tab_vision_config_params[2]; // authorized lost pairs percentage for tracking
  params.nb_tracker_width   = tab_vision_config_params[3]; // number of trackers in width of current picture
  params.nb_tracker_height  = tab_vision_config_params[4]; // number of trackers in height of current picture
  params.scale              = tab_vision_config_params[5]; // distance between two pixels in a pair
  params.trans_max          = tab_vision_config_params[6]; // largest value of trackers translation between two adjacent pictures
  params.max_pair_dist      = tab_vision_config_params[7]; // largest distance of pairs research from tracker location
  params.noise              = tab_vision_config_params[8]; // threshold of significative contrast

  ardrone_at_set_vision_track_params( &params );

  DEBUG_PRINT_SDK("CS %04d NB_P %04d Lossp %04d NB_Tlg %04d NB_TH %04d Scale %04d Dist_Max %04d Max_Dist %04d Noise %04d\n",
          tab_vision_config_params[0],
          tab_vision_config_params[1],
          tab_vision_config_params[2],
          tab_vision_config_params[3],
          tab_vision_config_params[4],
          tab_vision_config_params[5],
          tab_vision_config_params[6],
          tab_vision_config_params[7],
          tab_vision_config_params[8] );
}

static void ihm_RAWCapture(GtkWidget *widget, gpointer data)
{
  DEBUG_PRINT_SDK("   RAW video capture\n");

  ardrone_at_start_raw_capture();
}

static void ihm_Zapper(GtkWidget *widget, gpointer data)
{
  DEBUG_PRINT_SDK("   Zap\n");

  ardrone_at_zap(ZAP_CHANNEL_NEXT);
}

static void ihm_ImageButtonCB(GtkWidget *widget, gpointer data)
{
  int button = (int)data;

  DEBUG_PRINT_SDK("   Button clicked N°: %d\n", button);

  ardrone_at_set_vision_update_options(button);
}

// void ihm_ImageWinDestroy ( void )
void ihm_ImageWinDestroy( GtkWidget *widget, gpointer data )
{
  image_vision_window_status = WINDOW_CLOSED;
}

gint ihm_ImageWinDelete(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_show_image), FALSE);
  return FALSE;
}

static void ihm_showImage( gpointer pData )
{
  GtkWidget* widget = (GtkWidget*)pData;

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
    if( !GTK_IS_WIDGET(ihm_ImageWin) ) {
      create_image_window( );  // Recreate window if it has been killed
    }
    gtk_widget_show_all( ihm_ImageWin );
    image_vision_window_view = WINDOW_VISIBLE;
  }
  else {
    if( GTK_IS_WIDGET( ihm_ImageWin ) ) {
      gtk_widget_hide_all( ihm_ImageWin );
      image_vision_window_view = WINDOW_HIDE;
    }
  }
}

void create_image_window( void )
{
  /* Image display main window */
  /* ------------------------- */
  int k;
  GtkWidget *ihm_ImageVBox, *ihm_ImageVBoxPT, *ihm_ImageHBox[5], *ihm_ImageButton[11], *ihm_ImageLabel[11],  *ihm_ImageFrames[4];
  char  ihm_ImageTitle[128] = "VISION : Image" ;
  char *ihm_ImageFrameLabel[4]  = {"Vision states","Parametres tracking","Option tracking","Option calcul"};
  char *ihm_ImageEntryLabel[10]  = {  "      CS ",
                                     "           NB_P ",
                                     "       Loss% ",
                                     "     NB_TLg ",
                                     "   NB_TH ",
                                     " Scale ",
                                     "    Dist_Max ",
                                     "   Max_Dist ",
                                     "        Noise ",
                                     ""};
  char *ihm_ImageButtonLabel[11] = { "Update\nvision\nparams",
                                     " TZ_Known  ",
                                     "   No_SE    ",
                                     "   SE2      ",
                                     "     SE3     ",
                                     "Proj_OverScene",
                                     "    LS   ",
                                     " Frontal_Scene  ",
                                     " Flat_ground ",
                                     " RAW capture ",
                                     " Zapper"};

  // Image main window
  ihm_ImageWin = gtk_window_new( GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width(GTK_CONTAINER(ihm_ImageWin), 10);
  gtk_window_set_title(GTK_WINDOW(ihm_ImageWin), ihm_ImageTitle);
  gtk_signal_connect(GTK_OBJECT(ihm_ImageWin), "delete_event", G_CALLBACK(ihm_ImageWinDelete), NULL );
  gtk_signal_connect(GTK_OBJECT(ihm_ImageWin), "destroy", G_CALLBACK(ihm_ImageWinDestroy), NULL );

  // Boxes
  //ihm_VisionStateVBox = gtk_vbox_new(FALSE, 0);
  ihm_ImageVBox = gtk_vbox_new(FALSE, 0);
  ihm_Image_VBox = gtk_vbox_new(FALSE, 0);
  ihm_ImageVBoxPT = gtk_vbox_new(FALSE, 0);

  //hBox_vision_state = gtk_hbox_new(FALSE, 0);
  for (k=0; k<5; k++)  ihm_ImageHBox[k] = gtk_hbox_new(FALSE, 0);
  // Frames
  for (k=0; k<4; k++)  ihm_ImageFrames[k] = gtk_frame_new( ihm_ImageFrameLabel[k] );
  // Entries
  for (k=0; k<9; k++) {
    ihm_ImageEntry[k] = gtk_entry_new();
    gtk_widget_set_size_request(ihm_ImageEntry[k], 80, 20);
  }

  // Labels
  vp_os_memset(label_vision_state_value, 0, sizeof(label_vision_state_value));
  strcat(label_vision_state_value, "Not Connected");
  label_vision_values = (GtkLabel*) gtk_label_new(label_vision_state_value);

  gtk_container_add( GTK_CONTAINER(ihm_ImageFrames[0]), (GtkWidget*) label_vision_values );

  for (k=0; k<11; k++)  ihm_ImageLabel[k] = gtk_label_new( ihm_ImageEntryLabel[k] );
  // Buttons
  for (k=0; k<11; k++)  {
    ihm_ImageButton[k] = gtk_button_new_with_label( ihm_ImageButtonLabel[k] );
    if( k!= 0 && k!=9 && k!= 10 ) g_signal_connect( G_OBJECT(ihm_ImageButton[k]), "clicked", G_CALLBACK(ihm_ImageButtonCB), (gpointer)k );
    else if( k == 0 ) g_signal_connect( G_OBJECT(ihm_ImageButton[k]), "clicked", G_CALLBACK(ihm_sendVisionConfigParams), (gpointer)k );
    else if(k == 9) g_signal_connect( G_OBJECT(ihm_ImageButton[k]), "clicked", G_CALLBACK(ihm_RAWCapture), (gpointer)k );
    else g_signal_connect( G_OBJECT(ihm_ImageButton[k]), "clicked", G_CALLBACK(ihm_Zapper), (gpointer)k );
  }
  g_signal_connect(G_OBJECT(button_show_image), "clicked", G_CALLBACK(ihm_showImage), (gpointer)ihm_ImageWin );

  // Entries initial values
  char label_vision_default_val[10] ;
  tab_vision_config_params[0] = DEFAULT_CS;
  tab_vision_config_params[1] = DEFAULT_NB_PAIRS;
  tab_vision_config_params[2] = DEFAULT_LOSS_PER;
  tab_vision_config_params[3] = DEFAULT_NB_TRACKERS_WIDTH;
  tab_vision_config_params[4] = DEFAULT_NB_TRACKERS_HEIGHT;
  tab_vision_config_params[5] = DEFAULT_SCALE;
  tab_vision_config_params[6] = DEFAULT_TRANSLATION_MAX;
  tab_vision_config_params[7] = DEFAULT_MAX_PAIR_DIST;
  tab_vision_config_params[8] = DEFAULT_NOISE;

  for (k=0; k<9; k++)  {
    sprintf(label_vision_default_val, "%d", tab_vision_config_params[k]);
    gtk_entry_set_text( GTK_ENTRY(ihm_ImageEntry[k]), label_vision_default_val);
  }
  // Packing boxes
   //gtk_box_pack_start(GTK_BOX(ihm_VisionStateVBox),  label_vision_values, FALSE, FALSE, 0);
  for (k=0; k<5; k++)  {
    gtk_box_pack_start(GTK_BOX(ihm_ImageHBox[0]), ihm_ImageLabel[k], FALSE , FALSE, 0);
    gtk_box_pack_start(GTK_BOX(ihm_ImageHBox[0]), ihm_ImageEntry[k], FALSE , FALSE, 0);
  }
  for (k=5; k<9; k++)  {
    gtk_box_pack_start(GTK_BOX(ihm_ImageHBox[1]), ihm_ImageLabel[k], FALSE , FALSE, 0);
    gtk_box_pack_start(GTK_BOX(ihm_ImageHBox[1]), ihm_ImageEntry[k], FALSE , FALSE, 0);
  }
  gtk_box_pack_start(GTK_BOX(ihm_ImageHBox[1]), ihm_ImageLabel[9], FALSE , FALSE, 0); // To fill space

  gtk_box_pack_start(GTK_BOX(ihm_ImageVBoxPT), ihm_ImageHBox[0], FALSE , FALSE, 0);
  gtk_box_pack_start(GTK_BOX(ihm_ImageVBoxPT), ihm_ImageHBox[1], FALSE , FALSE, 0);

  //gtk_container_add( GTK_CONTAINER(ihm_ImageFrames[0]), hBox_vision_state );
  //gtk_box_pack_start(GTK_BOX(hBox_vision_state), ihm_ImageFrames[0], FALSE, FALSE, 0);
  //gtk_box_pack_start(GTK_BOX(hBox_vision_state), label_vision_values, FALSE, FALSE, 0);

  gtk_container_add(GTK_CONTAINER(ihm_ImageFrames[1]), ihm_ImageVBoxPT );

  gtk_box_pack_start(GTK_BOX(ihm_ImageHBox[2]), ihm_ImageFrames[1], FALSE , FALSE, 0);
  gtk_box_pack_start(GTK_BOX(ihm_ImageHBox[2]), ihm_ImageButton[0], TRUE  , FALSE, 5);
  gtk_box_pack_start(GTK_BOX(ihm_ImageHBox[2]), ihm_ImageButton[9], TRUE  , FALSE, 5);
  gtk_box_pack_start(GTK_BOX(ihm_ImageHBox[2]), ihm_ImageButton[10], TRUE  , FALSE, 5);

  for (k=1; k<5; k++)  gtk_box_pack_start(GTK_BOX( ihm_ImageHBox[3]), ihm_ImageButton[k], TRUE , FALSE, 0);
  for (k=5; k<9; k++)  gtk_box_pack_start(GTK_BOX( ihm_ImageHBox[4]), ihm_ImageButton[k], TRUE , FALSE, 0);
  gtk_container_add(GTK_CONTAINER( ihm_ImageFrames[2]), ihm_ImageHBox[3] );
  gtk_container_add(GTK_CONTAINER( ihm_ImageFrames[3]), ihm_ImageHBox[4] );

  gtk_box_pack_start(GTK_BOX(ihm_ImageVBox), ihm_ImageFrames[0], FALSE , FALSE, 5);
  gtk_box_pack_start(GTK_BOX(ihm_ImageVBox), ihm_ImageHBox[2]  , FALSE , FALSE, 5);
  gtk_box_pack_start(GTK_BOX(ihm_ImageVBox), ihm_ImageFrames[2], FALSE , FALSE, 5);
  gtk_box_pack_start(GTK_BOX(ihm_ImageVBox), ihm_ImageFrames[3], FALSE , FALSE, 5);
  gtk_box_pack_start(GTK_BOX(ihm_ImageVBox), ihm_Image_VBox    , FALSE , FALSE, 5);

  gtk_container_add(GTK_CONTAINER(ihm_ImageWin), ihm_ImageVBox);
  image_vision_window_view   = WINDOW_HIDE;
  image_vision_window_status = WINDOW_OPENED;

  // gtk_timeout_add(50, (GtkFunction)update_vision, NULL );
}

