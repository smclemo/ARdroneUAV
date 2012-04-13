/*
 * @ihm.c
 * @author florian.pantaleao.ext@parrot.fr
 * @date 2006/11/08
 *
 * ihm thread main source file
 * original version by Marc-Olivier DZEUKOU
 *
 */


#include   <stdio.h>
#include   <stdlib.h>
#include   <sys/types.h>
#include   <sys/fcntl.h>
#include   <fcntl.h>
#include   <termios.h>
#include   <time.h>
#include   <unistd.h>
#include   <errno.h>
#include   <curses.h>
#include   <pthread.h>
#include   <gtk/gtk.h>
#include   <locale.h>

#include <VP_Os/vp_os_malloc.h>
#include <VP_Os/vp_os_print.h>

#include <ardrone_api.h>

/*********** constants definition *****************/
//static const gchar* list_item_data_key = "color_key";

enum
{
    /* Text related columns */
    TEXT_C = 0,    /* Column with text strings */
    TEXT_VIS_C,    /* Visibility column for text strings */
    TEXT_COL_C,    /* Text color column */

    /* Image related columns */
    PIXBUF_C,     /* Column with GdkPixbufs */
    PIXBUF_VIS_C, /* Visibility column for pixbufs */

    /* Progress renderer related columns */
    PROGRESS_C,     /* Column with progress information [0, 100] */
    PROGRESS_VIS_C, /* Column with progress visibility */

    /* Last element of enumeration holds the number of columns */
    N_COLS
};


double round(double x); // l'include de math.h ne fonctionne pas

#include "common/common.h"
#include "common/mobile_config.h"
#include "UI/ui.h"

#define KIHM_ALLOC_SHARED_DATA
#include "ihm/ihm.h"
#include "ihm/view_drone_attitude.h"
#include "ihm/ihm_vision.h"

extern int exit_ihm_program;
extern C_RESULT signal_exit();

/* Vision image vars */
extern GtkWidget *ihm_ImageWin, *ihm_ImageEntry[9];
extern int tab_vision_config_params[10];
extern int vision_config_options;
extern int image_vision_window_status, image_vision_window_view;

GtkWidget *label_elapsedT;
GtkWidget *entry_PID[NB_ALL_GAINS], *entry_MiscVar[NB_MISC_VARS];
GtkWidget *button_ShowCurve[KIHM_NB_CURVES];
GtkWidget *button_show_image;
GtkWidget *label_mykonos_values[NB_ARDRONE_STATES];
GtkWidget *label_start_button_state, *darea_start_button_state;
GtkWidget *entry_ToyConfig[3];

/*Stephane*/
GtkWidget *nbDetectedTags_label;
GtkWidget *detectionHistory_label;
int detectionHistory[NB_DETECTION_SAMPLES];


int windows_status[KIHM_NB_CURVES];
int tab_g[NB_GAINS];
int tab_ag[NB_GAINS_ALT];
int tab_fp_g[NB_GAINS_FP];
extern int32_t  MiscVar[NB_MISC_VARS];
mobile_config_t *pcfg;

char label_mykonos_state_value[32];
char label_ctrl_state_value[32];

typedef struct {
   GtkWidget *duration;
   GtkWidget *freq;
}private_data_led;

ihm_time_t ihm_time;

int32_t ihm_start = 0;

void ihm_set_start_button_state( int32_t start )
{
  ihm_start = start;
}

static void ihm_sendGains(GtkWidget *widget, gpointer data)
{
  api_control_gains_t gains;

  DEBUG_PRINT_SDK("PQ_Kp = %04d\tR_Kp = %04d\tR_Ki = %04d\tEA_Kp = %04d\tEA_Ki = %04d\tALT_KP = %04d\
                  \tVZ_KP = %04d\tVz_KI = %04d\tALT_TD = %04d\tFP_KP = %04d\tFP_KI = %04d\n",
                  tab_g[0], tab_g[1], tab_g[2], tab_g[3], tab_g[4],
                  tab_ag[0], tab_ag[1], tab_ag[2], tab_ag[3],
                  tab_fp_g[0], tab_fp_g[1]);

  gains.pq_kp       = tab_g[0];
  gains.r_kp        = tab_g[1];
  gains.r_ki        = tab_g[2];
  gains.ea_kp       = tab_g[3];
  gains.ea_ki       = tab_g[4];
  gains.alt_kp      = tab_ag[0];
  gains.alt_ki      = tab_ag[1];
  gains.vz_kp      = tab_ag[2];
  gains.vz_ki      = tab_ag[3];
  gains.hovering_kp = tab_fp_g[0];
  gains.hovering_ki = tab_fp_g[1];

  ardrone_at_set_control_gains(&gains);
}

static void ihm_sendFlatTrim(GtkWidget *widget, gpointer data)
{
    ardrone_at_set_flat_trim();
}

static void ihm_setConfigParams(GtkWidget *widget, gpointer data)
{
  GtkWidget* diag;
  char* param = (char*) gtk_entry_get_text((GtkEntry*)entry_ToyConfig[0]);
  char* value = (char*) gtk_entry_get_text((GtkEntry*)entry_ToyConfig[1]);

  if((strlen(param) != 0) && (strlen(value) != 0))
  {
    DEBUG_PRINT_SDK("Setting configuration: Param:%s Value:%s\n", param, value);

    ardrone_at_set_toy_configuration(param, value);
  }
  else
  {
    diag = gtk_message_dialog_new (NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_WARNING, GTK_BUTTONS_OK, "Not all values have been defined");
    gtk_dialog_run(GTK_DIALOG(diag));
    gtk_widget_destroy(diag);
  }
}

static void ihm_setConfigParamsWindow(GtkWidget *widget, gpointer data)
{
  GtkWidget *MainWin, *hBox, *vBox;
  GtkWidget *label_param, *label_value /*, *label_description*/ ;
  GtkWidget *button_set;

  MainWin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(MainWin), GTK_WIN_POS_CENTER_ALWAYS);
  gtk_container_set_border_width(GTK_CONTAINER(MainWin), KIHM_BORDER);
  gtk_window_set_title(GTK_WINDOW(MainWin), "Toy configuration");

  vBox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(MainWin), vBox);

  hBox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vBox), hBox  , TRUE, TRUE, 5);

  label_param = gtk_label_new("Parameter:");
  label_value = gtk_label_new("Value:");
//   label_description = gtk_label_new("Description:");
  entry_ToyConfig[0] = gtk_entry_new();
  entry_ToyConfig[1] = gtk_entry_new();
//   entry_ToyConfig[2] = gtk_entry_new();

  gtk_box_pack_start(GTK_BOX(hBox), label_param  , TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(hBox), entry_ToyConfig[0]  , TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(hBox), label_value  , TRUE, TRUE, 2);
  gtk_box_pack_start(GTK_BOX(hBox), entry_ToyConfig[1]  , TRUE, TRUE, 2);
//   gtk_box_pack_start(GTK_BOX(hBox), label_description  , TRUE, TRUE, 2);
//   gtk_box_pack_start(GTK_BOX(hBox), entry_ToyConfig[2]  , TRUE, TRUE, 2);

  button_set = gtk_button_new_with_label("Set configuration to Toy");

  g_signal_connect(G_OBJECT(button_set), "clicked", G_CALLBACK(ihm_setConfigParams), NULL);

  gtk_box_pack_start(GTK_BOX(vBox), button_set  , TRUE, TRUE, 0);

  gtk_widget_show_all(MainWin);
}

static void ihm_freezeCurves(GtkWidget *widget, gpointer data)
{
  ihm_freeze_curves = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
}

static void ihm_showCurves(GtkWidget *widget, gpointer data)
{
  int curve = (int) data; int k;
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) {
    if( !GTK_IS_WIDGET(ihm_CA[curve].PrivateWin) )  {
      ihm_initCurves( ihm_CA[curve].PrivateWin, curve); // Recreate window if it has been killed
      windows_status[curve] = WINDOW_OPENED;
    }
    gtk_widget_show_all(ihm_CA[curve].PrivateWin);
    for (k=0;k<ihm_CA[curve].nb_range;k++)
      if (k != ihm_CA[curve].range) gtk_widget_hide(ihm_CA[curve].tSA[k].vBox_Grad); // Hide unchoosed graduation
    ihm_CA[curve].win_view = WINDOW_VISIBLE;
  }
  else {
    if( GTK_IS_WIDGET(ihm_CA[curve].PrivateWin) ) {
      gtk_widget_hide(ihm_CA[curve].PrivateWin);
      ihm_CA[curve].win_view = WINDOW_HIDE;
    }
  }
}

static void ihm_plusScale(GtkWidget *widget, gpointer data)
{
  SIHM_CurveAttributes *pCA = (SIHM_CurveAttributes*) data;
  if (pCA->range < pCA->nb_range-1) {
    gtk_widget_hide_all(pCA->tSA[pCA->range].vBox_Grad);
    pCA->range++;
    gtk_widget_show_all(pCA->tSA[pCA->range].vBox_Grad);
  }
}

static void ihm_minusScale(GtkWidget *widget, gpointer data)
{
  SIHM_CurveAttributes *pCA = (SIHM_CurveAttributes*) data;
  if (pCA->range > 0) {
    gtk_widget_hide_all(pCA->tSA[pCA->range].vBox_Grad);
    pCA->range--;
    gtk_widget_show_all(pCA->tSA[pCA->range].vBox_Grad);
  }
}

void ihm_private_window_destroy( SIHM_CurveAttributes *pCA )
{
  int k;
  for(k=0;k<KIHM_NB_CURVES;k++) {
    if( (windows_status[k] == WINDOW_OPENED) && (!GTK_IS_WIDGET(ihm_CA[k].PrivateWin)) ) {
      windows_status[k] = WINDOW_CLOSED;
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_ShowCurve[k]), FALSE );
    }
  }
}

void ihm_main_quit( void )
{
  if( image_vision_window_status != WINDOW_CLOSED )
    gtk_widget_destroy(ihm_ImageWin);

  signal_exit();

  gtk_main_quit();
}

void ihm_initCA(SIHM_CurveAttributes *pCA, char* title_str, char* ctrl_lbl, int nb_val, int nb_range)
{
  /* initialize a SIHM_CurveAttributes structure */
  int k;

  strncpy(pCA->title, title_str, sizeof(pCA->title));
  pCA->title[sizeof(pCA->title)-1] = 0;
  strncpy(pCA->ctrl_lbl, ctrl_lbl, sizeof(pCA->ctrl_lbl));
  pCA->ctrl_lbl[sizeof(pCA->ctrl_lbl)-1] = 0;
  pCA->show = TRUE;
  pCA->nb_val = nb_val;
  pCA->legend = (char**) vp_os_malloc(nb_val * sizeof(char*));
  for (k=0; k<nb_val; k++) pCA->legend[k] = (char*) vp_os_malloc(32);
  pCA->GC = (GdkGC**) vp_os_malloc(nb_val * sizeof(GdkGC*));
  pCA->tval = (double**) vp_os_malloc(nb_val * sizeof(double*));
  for (k=0; k<nb_val; k++) pCA->tval[k] = (double*) vp_os_malloc(KIHM_N_PT2PLOT*sizeof(double));
  pCA->nb_range = nb_range;
  pCA->range = pCA->nb_range-1;
  pCA->tSA = (SIHM_ScaleAttributes*) vp_os_malloc(nb_range*sizeof(SIHM_ScaleAttributes));

  pCA->PrivateWin = gtk_window_new( GTK_WINDOW_TOPLEVEL);
  gtk_window_set_position(GTK_WINDOW(pCA->PrivateWin), GTK_WIN_POS_CENTER_ALWAYS);
  gtk_window_set_title(GTK_WINDOW(pCA->PrivateWin), title_str);
  gtk_signal_connect(GTK_OBJECT(pCA->PrivateWin), "destroy", G_CALLBACK(ihm_private_window_destroy), (gpointer)pCA );
  pCA->PrivateVBox = gtk_vbox_new(FALSE, 0);
  gtk_container_add(GTK_CONTAINER(pCA->PrivateWin), pCA->PrivateVBox);
  pCA->win_view = WINDOW_HIDE;
}


void ihm_createGCs(GtkWidget *Drawable)
{
  /* Graphic contexts definition */
  /* --------------------------- */
  GdkColor Color;

  // Red
  Color.red   = 0xFFFF;
  Color.green = 0;
  Color.blue  = 0;
  if (gdk_colormap_alloc_color(gdk_colormap_get_system(), &Color, FALSE, TRUE) == FALSE)
    perror("gdk_colormap_alloc_color");
  ihm_GC[KIHM_GC_RED] = gdk_gc_new(Drawable->window);
  gdk_gc_set_foreground(ihm_GC[KIHM_GC_RED], &Color);

  // light Red
  Color.red   = 0xCFFF;
  Color.green = 0;
  Color.blue  = 0;
  if (gdk_colormap_alloc_color(gdk_colormap_get_system(), &Color, FALSE, TRUE) == FALSE)
    perror("gdk_colormap_alloc_color");
  ihm_GC[KIHM_GC_LIGHTRED] = gdk_gc_new(Drawable->window);
  gdk_gc_set_foreground(ihm_GC[KIHM_GC_LIGHTRED], &Color);

  // Green
  Color.red   = 0;
  Color.green = 0xDFFF;
  Color.blue  = 0;
  if (gdk_colormap_alloc_color(gdk_colormap_get_system(), &Color, FALSE, TRUE) == FALSE)
    perror("gdk_colormap_alloc_color");
  ihm_GC[KIHM_GC_GREEN] = gdk_gc_new(Drawable->window);
  gdk_gc_set_foreground(ihm_GC[KIHM_GC_GREEN], &Color);

  // Blue
  Color.red   = 0;
  Color.green = 0;
  Color.blue  = 0xFFFF;
  if (gdk_colormap_alloc_color(gdk_colormap_get_system(), &Color, FALSE, TRUE) == FALSE)
    perror("gdk_colormap_alloc_color");
  ihm_GC[KIHM_GC_BLUE] = gdk_gc_new(Drawable->window);
  gdk_gc_set_foreground(ihm_GC[KIHM_GC_BLUE], &Color);

  // Grey
  Color.red   = 0xD000;
  Color.green = 0xD000;
  Color.blue  = 0xD000;
  if (gdk_colormap_alloc_color(gdk_colormap_get_system(), &Color, FALSE, TRUE) == FALSE)
    perror("gdk_colormap_alloc_color");
  ihm_GC[KIHM_GC_GREY] = gdk_gc_new(Drawable->window);
  gdk_gc_set_foreground(ihm_GC[KIHM_GC_GREY], &Color);

  // Light blue
  Color.red   = 0;
  Color.green = 0;
  Color.blue  = 0xF000;
  if (gdk_colormap_alloc_color(gdk_colormap_get_system(), &Color, FALSE, TRUE) == FALSE)
    perror("gdk_colormap_alloc_color");
  ihm_GC[KIHM_GC_LIGHTBLUE] = gdk_gc_new(Drawable->window);
  gdk_gc_set_foreground(ihm_GC[KIHM_GC_LIGHTBLUE], &Color);

  // Light grey
  Color.red   = 0xF000;
  Color.green = 0xF000;
  Color.blue  = 0xF000;
  if (gdk_colormap_alloc_color(gdk_colormap_get_system(), &Color, FALSE, TRUE) == FALSE)
    perror("gdk_colormap_alloc_color");
  ihm_GC[KIHM_GC_LIGHTGREY] = gdk_gc_new(Drawable->window);
  gdk_gc_set_foreground(ihm_GC[KIHM_GC_LIGHTGREY], &Color);

  // Light grey dashed lines
  int LineWidth = 1;
  ihm_GC[KIHM_GC_DASHLINE] = gdk_gc_new(Drawable->window);
  gdk_gc_set_foreground(ihm_GC[KIHM_GC_DASHLINE], &Color);
  gdk_gc_set_line_attributes(ihm_GC[KIHM_GC_DASHLINE],
			     LineWidth,
			     GDK_LINE_ON_OFF_DASH,
			     GDK_CAP_NOT_LAST,
			     GDK_JOIN_BEVEL);

  // Black for individual window GC
  Color.red   = 0x0000;
  Color.green = 0x0000;
  Color.blue  = 0x0000;
  if (gdk_colormap_alloc_color(gdk_colormap_get_system(), &Color, FALSE, TRUE) == FALSE)
    perror("gdk_colormap_alloc_color");
  ihm_GC[KIHM_GC_BLACK] = gdk_gc_new(Drawable->window);
  gdk_gc_set_foreground(ihm_GC[KIHM_GC_BLACK], &Color);

  // White for individual window GC
  Color.red   = 0xFFFF;
  Color.green = 0xFFFF;
  Color.blue  = 0xFFFF;
  if (gdk_colormap_alloc_color(gdk_colormap_get_system(), &Color, FALSE, TRUE) == FALSE)
    perror("gdk_colormap_alloc_color");
  ihm_GC[KIHM_GC_WHITE] = gdk_gc_new(Drawable->window);
  gdk_gc_set_foreground(ihm_GC[KIHM_GC_WHITE], &Color);
}


void ihm_createCurve(SIHM_CurveAttributes *pCA)
{
  /* Creation of widget hierarchy for curve display */
  /* ---------------------------------------------- */
  int k,i;
  GtkWidget *frame, *vBox, *hBox_LegendValue, *hBox_Curve, *vBox_Ctrl;
  GtkWidget *button_PlusScale, *button_MinusScale, *label_CtrlScale;
  GdkGCValues GCval;
  GdkColor legend_color;
  static PangoFontDescription* pPFD = NULL;

  // one-time font allocation
  if (pPFD == NULL)
    pPFD = pango_font_description_from_string("terminal 12"); // select a standard fixed font

  // Legend and value labels
  pCA->label_Legend = (GtkWidget**) vp_os_malloc(pCA->nb_val*sizeof(GtkWidget*));
  pCA->lblVal  = (GtkWidget**) vp_os_malloc(pCA->nb_val*sizeof(GtkWidget*));
  for (k=0; k<pCA->nb_val; k++) {
    pCA->label_Legend[k] = gtk_label_new(pCA->legend[k]);
    gtk_widget_modify_font(pCA->label_Legend[k], pPFD);
    // retrieve foreground colors of GC in 3 steps
    //  1) retrieve GC values but only pixel field of foreground is set
    //  2) retrieve rgb components with gdk_colormap_query_color
    //  3) set foreground color with gtk_widget_modify_fg
    gdk_gc_get_values(pCA->GC[k], &GCval);
    gdk_colormap_query_color(gdk_gc_get_colormap(pCA->GC[k]),
			     GCval.foreground.pixel,
			     &legend_color);
    gtk_widget_modify_fg(pCA->label_Legend[k], GTK_STATE_NORMAL, &legend_color);
    pCA->lblVal[k] = gtk_label_new("...");
    gtk_widget_modify_font(pCA->lblVal[k], pPFD);
  }

  for (i=0;i<pCA->nb_range;i++) {
    pCA->tSA[i].vBox_Grad = gtk_vbox_new(FALSE, 0);

    pCA->tSA[i].phys_range = pCA->tSA[i].phys_max - pCA->tSA[i].phys_min;
    pCA->tSA[i].nb_grad = (int) round(pCA->tSA[i].phys_range / pCA->tSA[i].phys_step) + 1;
    pCA->tSA[i].y_orig = KIHM_DAREA_CURVE_Y_OFFSET +
      (int) ((pCA->tSA[i].phys_max * KIHM_DAREA_CURVE_Y_SIZE) / pCA->tSA[i].phys_range);
    pCA->tSA[i].y_min = pCA->tSA[i].y_orig -
      (int) (((pCA->tSA[i].phys_min - pCA->tSA[i].phys_orig) * KIHM_DAREA_CURVE_Y_SIZE) / pCA->tSA[i].phys_range);
    pCA->tSA[i].y_max = pCA->tSA[i].y_orig -
      (int) (((pCA->tSA[i].phys_max - pCA->tSA[i].phys_orig) * KIHM_DAREA_CURVE_Y_SIZE) / pCA->tSA[i].phys_range);

    // Graduation labels and empty boxes to separate graduations regularly
    char grad_str[20];
    GtkWidget *label_Grad, *hbox_InterGrad;

    for (k=0; k<pCA->tSA[i].nb_grad-1; k++) {
      sprintf(grad_str, pCA->tSA[i].grad_format, pCA->tSA[i].phys_min + k*pCA->tSA[i].phys_step);
      label_Grad = gtk_label_new(grad_str);
      hbox_InterGrad = gtk_hbox_new(FALSE,0);
      gtk_box_pack_end(GTK_BOX(pCA->tSA[i].vBox_Grad), label_Grad    , FALSE, FALSE, 0);
      gtk_box_pack_end(GTK_BOX(pCA->tSA[i].vBox_Grad), hbox_InterGrad,  TRUE, FALSE, 0);
    }
    k = pCA->tSA[i].nb_grad-1;
    sprintf(grad_str, pCA->tSA[i].grad_format, pCA->tSA[i].phys_min + k*pCA->tSA[i].phys_step);
    label_Grad = gtk_label_new(grad_str);
    gtk_box_pack_end(GTK_BOX(pCA->tSA[i].vBox_Grad), label_Grad, FALSE, FALSE, 0);
  }

  // scale control
  GList* children_list;
  button_PlusScale = gtk_button_new_with_label("+");
  children_list = gtk_container_get_children(GTK_CONTAINER(button_PlusScale));
  gtk_label_set_justify(GTK_LABEL(children_list->data), GTK_JUSTIFY_CENTER);
  g_signal_connect(G_OBJECT(button_PlusScale), "clicked", G_CALLBACK(ihm_plusScale), (gpointer)pCA);
  button_MinusScale = gtk_button_new_with_label("-");
  children_list = gtk_container_get_children(GTK_CONTAINER(button_MinusScale));
  gtk_label_set_justify(GTK_LABEL(children_list->data), GTK_JUSTIFY_CENTER);
  g_signal_connect(G_OBJECT(button_MinusScale), "clicked", G_CALLBACK(ihm_minusScale), (gpointer)pCA);
  label_CtrlScale = gtk_label_new("Scale\nadjust");

  // Creation of other widgets
  frame = gtk_frame_new(pCA->title);
  vBox = gtk_vbox_new(FALSE, 0);
  hBox_LegendValue = gtk_hbox_new(FALSE, 0);
  hBox_Curve = gtk_hbox_new(FALSE, 0);
  vBox_Ctrl = gtk_vbox_new(FALSE, 0);
  pCA->DA = gtk_drawing_area_new();
  gtk_drawing_area_size((GtkDrawingArea*)pCA->DA,
			KIHM_DAREA_CURVE_X_SIZE,
			KIHM_DAREA_CURVE_Y_SIZE+2*KIHM_DAREA_CURVE_Y_OFFSET);

  // Creation of widget hierarchy
  gtk_box_pack_start(GTK_BOX(vBox_Ctrl), button_PlusScale , FALSE, FALSE, 5);
  gtk_box_pack_start(GTK_BOX(vBox_Ctrl), label_CtrlScale  , FALSE, FALSE, 5);
  gtk_box_pack_start(GTK_BOX(vBox_Ctrl), button_MinusScale, FALSE, FALSE, 5);

  for (k=0; k<pCA->nb_val; k++) {
    gtk_box_pack_start(GTK_BOX(hBox_LegendValue), pCA->label_Legend[k], FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hBox_LegendValue), pCA->lblVal[k] , FALSE, FALSE, 0);
  }

  gtk_box_pack_end(GTK_BOX(hBox_Curve), pCA->DA  , FALSE, FALSE, 0);
  for (i=0;i<pCA->nb_range;i++)
    gtk_box_pack_end(GTK_BOX(hBox_Curve), pCA->tSA[i].vBox_Grad,  TRUE,  TRUE, 0);
  gtk_box_pack_end(GTK_BOX(hBox_Curve), vBox_Ctrl,  TRUE,  TRUE, 0);

  gtk_box_pack_start(GTK_BOX(vBox), hBox_LegendValue, FALSE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vBox), hBox_Curve      , FALSE, TRUE, 0);
  gtk_container_add(GTK_CONTAINER(frame), vBox);
  pCA->top = frame;
  gtk_box_pack_start(GTK_BOX(pCA->PrivateVBox), pCA->top, FALSE, TRUE, 5 );
}


void ihm_destroyCurves()
{
  int i;
  for(i = 0 ; i < KIHM_NB_CURVES ; i++)
    {
      vp_os_free(ihm_CA[i].label_Legend);
      vp_os_free(ihm_CA[i].lblVal);
    }
}


void ihm_initCurves(GtkWidget *widget, int curve)
{
  int scale_idx;

  switch( curve )
  {
    /* Angular rates display */
    /* --------------------- */
  case KIHM_CURVE_GYRO:
    ihm_initCA(&ihm_CA[KIHM_CURVE_GYRO], GYRO_WIN_TITLE, "Angular rates", 4, 4);
    strcpy(ihm_CA[KIHM_CURVE_GYRO].legend[0], "Gyro X (p)");
    strcpy(ihm_CA[KIHM_CURVE_GYRO].legend[1], "Gyro Y (q)");
    strcpy(ihm_CA[KIHM_CURVE_GYRO].legend[2], "Gyro Z (r)");
    strcpy(ihm_CA[KIHM_CURVE_GYRO].legend[3], "Table");
    ihm_CA[KIHM_CURVE_GYRO].GC[0] = ihm_GC[KIHM_GC_BLUE];
    ihm_CA[KIHM_CURVE_GYRO].GC[1] = ihm_GC[KIHM_GC_GREEN];
    ihm_CA[KIHM_CURVE_GYRO].GC[2] = ihm_GC[KIHM_GC_RED];
    ihm_CA[KIHM_CURVE_GYRO].GC[3] = ihm_GC[KIHM_GC_BLACK];
    scale_idx = 0;
    ihm_CA[KIHM_CURVE_GYRO].tSA[scale_idx].phys_orig =    0.0;
    ihm_CA[KIHM_CURVE_GYRO].tSA[scale_idx].phys_min  =   -1.0;
    ihm_CA[KIHM_CURVE_GYRO].tSA[scale_idx].phys_max  =    1.0;
    ihm_CA[KIHM_CURVE_GYRO].tSA[scale_idx].phys_step =    0.2;
    strcpy(ihm_CA[KIHM_CURVE_GYRO].tSA[scale_idx].grad_format, "% 6.1f");
    scale_idx++;
    ihm_CA[KIHM_CURVE_GYRO].tSA[scale_idx].phys_orig =    0.0;
    ihm_CA[KIHM_CURVE_GYRO].tSA[scale_idx].phys_min  =   -5.0;
    ihm_CA[KIHM_CURVE_GYRO].tSA[scale_idx].phys_max  =    5.0;
    ihm_CA[KIHM_CURVE_GYRO].tSA[scale_idx].phys_step =    1.0;
    strcpy(ihm_CA[KIHM_CURVE_GYRO].tSA[scale_idx].grad_format, "% 6.1f");
    scale_idx++;
    ihm_CA[KIHM_CURVE_GYRO].tSA[scale_idx].phys_orig =    0.0;
    ihm_CA[KIHM_CURVE_GYRO].tSA[scale_idx].phys_min  =  -20.0;
    ihm_CA[KIHM_CURVE_GYRO].tSA[scale_idx].phys_max  =   20.0;
    ihm_CA[KIHM_CURVE_GYRO].tSA[scale_idx].phys_step =    5.0;
    strcpy(ihm_CA[KIHM_CURVE_GYRO].tSA[scale_idx].grad_format, "% 6.1f");
    scale_idx++;
    ihm_CA[KIHM_CURVE_GYRO].tSA[scale_idx].phys_orig =    0.0;
    ihm_CA[KIHM_CURVE_GYRO].tSA[scale_idx].phys_min  = -100.0;
    ihm_CA[KIHM_CURVE_GYRO].tSA[scale_idx].phys_max  =  100.0;
    ihm_CA[KIHM_CURVE_GYRO].tSA[scale_idx].phys_step =   20.0;
    strcpy(ihm_CA[KIHM_CURVE_GYRO].tSA[scale_idx].grad_format, "% 6.1f");
    strcpy(ihm_CA[KIHM_CURVE_GYRO].val_format , "% 7.2f");
    ihm_createCurve(&ihm_CA[KIHM_CURVE_GYRO]);
    break;


    /* Accelerations display */
    /* --------------------- */
  case KIHM_CURVE_ACC:
    ihm_initCA(&ihm_CA[KIHM_CURVE_ACC], ACC_WIN_TITLE , "Accelerations", 3, 2);
    strcpy(ihm_CA[KIHM_CURVE_ACC].legend[0], "Acc X");
    strcpy(ihm_CA[KIHM_CURVE_ACC].legend[1], "Acc Y");
    strcpy(ihm_CA[KIHM_CURVE_ACC].legend[2], "Acc Z");
    ihm_CA[KIHM_CURVE_ACC].GC[0] = ihm_GC[KIHM_GC_BLUE];
    ihm_CA[KIHM_CURVE_ACC].GC[1] = ihm_GC[KIHM_GC_GREEN];
    ihm_CA[KIHM_CURVE_ACC].GC[2] = ihm_GC[KIHM_GC_RED];
    scale_idx = 0;
    ihm_CA[KIHM_CURVE_ACC].tSA[scale_idx].phys_orig =  0.0;
    ihm_CA[KIHM_CURVE_ACC].tSA[scale_idx].phys_min  = -0.2;
    ihm_CA[KIHM_CURVE_ACC].tSA[scale_idx].phys_max  =  0.2;
    ihm_CA[KIHM_CURVE_ACC].tSA[scale_idx].phys_step =  0.05;
    strcpy(ihm_CA[KIHM_CURVE_ACC].tSA[scale_idx].grad_format, "% 5.2f");
    scale_idx++;
    ihm_CA[KIHM_CURVE_ACC].tSA[scale_idx].phys_orig =  0.0;
    ihm_CA[KIHM_CURVE_ACC].tSA[scale_idx].phys_min  = -1.5;
    ihm_CA[KIHM_CURVE_ACC].tSA[scale_idx].phys_max  =  1.5;
    ihm_CA[KIHM_CURVE_ACC].tSA[scale_idx].phys_step =  0.5;
    strcpy(ihm_CA[KIHM_CURVE_ACC].tSA[scale_idx].grad_format, "% 4.1f");
    strcpy(ihm_CA[KIHM_CURVE_ACC].val_format , "% 6.3f");
    ihm_createCurve(&ihm_CA[KIHM_CURVE_ACC]);
    break;

    /* theta (pitch) display */
    /* --------------------- */
    // there are 4 different values: computed with acc or gyros or both and reference
    // they are ordered in ihm_CA[KIHM_CURVE_THETA] as follow
    //   1) fused values
    //   2) reference
    //   3) accelerometers
    //   4) gyrometers
    // 3 and 4 may be optionnal
  case KIHM_CURVE_THETA:
    ihm_initCA(&ihm_CA[KIHM_CURVE_THETA], THETA_WIN_TITLE, "Theta", 6, 4);
    strcpy(ihm_CA[KIHM_CURVE_THETA].legend[0], "Fus");
    strcpy(ihm_CA[KIHM_CURVE_THETA].legend[1], "Ref");
    strcpy(ihm_CA[KIHM_CURVE_THETA].legend[2], "Acc");
    strcpy(ihm_CA[KIHM_CURVE_THETA].legend[3], "Gyr");
    strcpy(ihm_CA[KIHM_CURVE_THETA].legend[4], "Table");
    strcpy(ihm_CA[KIHM_CURVE_THETA].legend[5], "POLARIS");
    ihm_CA[KIHM_CURVE_THETA].GC[0] = ihm_GC[KIHM_GC_RED];
    ihm_CA[KIHM_CURVE_THETA].GC[1] = ihm_GC[KIHM_GC_BLACK];
    ihm_CA[KIHM_CURVE_THETA].GC[2] = ihm_GC[KIHM_GC_BLUE];
    ihm_CA[KIHM_CURVE_THETA].GC[3] = ihm_GC[KIHM_GC_GREEN];
    ihm_CA[KIHM_CURVE_THETA].GC[4] = ihm_GC[KIHM_GC_LIGHTRED];
    ihm_CA[KIHM_CURVE_THETA].GC[5] = ihm_GC[KIHM_GC_LIGHTBLUE];
    scale_idx = 0;
    ihm_CA[KIHM_CURVE_THETA].tSA[scale_idx].phys_orig =   0.0;
    ihm_CA[KIHM_CURVE_THETA].tSA[scale_idx].phys_min  =  -1.0;
    ihm_CA[KIHM_CURVE_THETA].tSA[scale_idx].phys_max  =   1.0;
    ihm_CA[KIHM_CURVE_THETA].tSA[scale_idx].phys_step =   0.2;
    strcpy(ihm_CA[KIHM_CURVE_THETA].tSA[scale_idx].grad_format, "% 5.1f");
    scale_idx++;
    ihm_CA[KIHM_CURVE_THETA].tSA[scale_idx].phys_orig =   0.0;
    ihm_CA[KIHM_CURVE_THETA].tSA[scale_idx].phys_min  =  -5.0;
    ihm_CA[KIHM_CURVE_THETA].tSA[scale_idx].phys_max  =   5.0;
    ihm_CA[KIHM_CURVE_THETA].tSA[scale_idx].phys_step =   1.0;
    strcpy(ihm_CA[KIHM_CURVE_THETA].tSA[scale_idx].grad_format, "% 5.1f");
    scale_idx++;
    ihm_CA[KIHM_CURVE_THETA].tSA[scale_idx].phys_orig =   0.0;
    ihm_CA[KIHM_CURVE_THETA].tSA[scale_idx].phys_min  = -15.0;
    ihm_CA[KIHM_CURVE_THETA].tSA[scale_idx].phys_max  =  15.0;
    ihm_CA[KIHM_CURVE_THETA].tSA[scale_idx].phys_step =   3.0;
    strcpy(ihm_CA[KIHM_CURVE_THETA].tSA[scale_idx].grad_format, "% 5.1f");
    scale_idx++;
    ihm_CA[KIHM_CURVE_THETA].tSA[scale_idx].phys_orig =   0.0;
    ihm_CA[KIHM_CURVE_THETA].tSA[scale_idx].phys_min  = -45.0;
    ihm_CA[KIHM_CURVE_THETA].tSA[scale_idx].phys_max  =  45.0;
    ihm_CA[KIHM_CURVE_THETA].tSA[scale_idx].phys_step =  15.0;
    strcpy(ihm_CA[KIHM_CURVE_THETA].tSA[scale_idx].grad_format, "% 5.1f");
    strcpy(ihm_CA[KIHM_CURVE_THETA].val_format , "% 7.2f");
    ihm_createCurve(&ihm_CA[KIHM_CURVE_THETA]);
    printf("   ihm_init : theta display\n");
    break;

    /* phi (roll) display */
    /* ------------------ */
    // there are 4 different values: computed with acc or gyros or both and reference (see theta comment)
  case KIHM_CURVE_PHI:
    ihm_initCA(&ihm_CA[KIHM_CURVE_PHI], PHI_WIN_TITLE, "Phi", 6, 4);
    strcpy(ihm_CA[KIHM_CURVE_PHI].legend[0], "Fus");
    strcpy(ihm_CA[KIHM_CURVE_PHI].legend[1], "Ref");
    strcpy(ihm_CA[KIHM_CURVE_PHI].legend[2], "Acc");
    strcpy(ihm_CA[KIHM_CURVE_PHI].legend[3], "Gyr");
    strcpy(ihm_CA[KIHM_CURVE_PHI].legend[4], "Table");
    strcpy(ihm_CA[KIHM_CURVE_PHI].legend[5], "POLARIS");
    ihm_CA[KIHM_CURVE_PHI].GC[0] = ihm_GC[KIHM_GC_RED];
    ihm_CA[KIHM_CURVE_PHI].GC[1] = ihm_GC[KIHM_GC_BLACK];
    ihm_CA[KIHM_CURVE_PHI].GC[2] = ihm_GC[KIHM_GC_BLUE];
    ihm_CA[KIHM_CURVE_PHI].GC[3] = ihm_GC[KIHM_GC_GREEN];
    ihm_CA[KIHM_CURVE_PHI].GC[4] = ihm_GC[KIHM_GC_LIGHTRED];
    ihm_CA[KIHM_CURVE_PHI].GC[5] = ihm_GC[KIHM_GC_LIGHTBLUE];
    scale_idx = 0;
    ihm_CA[KIHM_CURVE_PHI].tSA[scale_idx].phys_orig =   0.0;
    ihm_CA[KIHM_CURVE_PHI].tSA[scale_idx].phys_min  =  -1.0;
    ihm_CA[KIHM_CURVE_PHI].tSA[scale_idx].phys_max  =   1.0;
    ihm_CA[KIHM_CURVE_PHI].tSA[scale_idx].phys_step =   0.2;
    strcpy(ihm_CA[KIHM_CURVE_PHI].tSA[scale_idx].grad_format, "% 5.1f");
    scale_idx++;
    ihm_CA[KIHM_CURVE_PHI].tSA[scale_idx].phys_orig =   0.0;
    ihm_CA[KIHM_CURVE_PHI].tSA[scale_idx].phys_min  =  -5.0;
    ihm_CA[KIHM_CURVE_PHI].tSA[scale_idx].phys_max  =   5.0;
    ihm_CA[KIHM_CURVE_PHI].tSA[scale_idx].phys_step =   1.0;
    strcpy(ihm_CA[KIHM_CURVE_PHI].tSA[scale_idx].grad_format, "% 5.1f");
    scale_idx++;
    ihm_CA[KIHM_CURVE_PHI].tSA[scale_idx].phys_orig =   0.0;
    ihm_CA[KIHM_CURVE_PHI].tSA[scale_idx].phys_min  = -15.0;
    ihm_CA[KIHM_CURVE_PHI].tSA[scale_idx].phys_max  =  15.0;
    ihm_CA[KIHM_CURVE_PHI].tSA[scale_idx].phys_step =   3.0;
    strcpy(ihm_CA[KIHM_CURVE_PHI].tSA[scale_idx].grad_format, "% 5.1f");
    scale_idx++;
    ihm_CA[KIHM_CURVE_PHI].tSA[scale_idx].phys_orig =   0.0;
    ihm_CA[KIHM_CURVE_PHI].tSA[scale_idx].phys_min  = -45.0;
    ihm_CA[KIHM_CURVE_PHI].tSA[scale_idx].phys_max  =  45.0;
    ihm_CA[KIHM_CURVE_PHI].tSA[scale_idx].phys_step =  15.0;
    strcpy(ihm_CA[KIHM_CURVE_PHI].tSA[scale_idx].grad_format, "% 5.1f");
    strcpy(ihm_CA[KIHM_CURVE_PHI].val_format , "% 7.2f");
    ihm_createCurve(&ihm_CA[KIHM_CURVE_PHI]);
    printf("   ihm_init : phi display\n");
    break;

  /* vbat display */
  /* ------------ */
  case KIHM_CURVE_VBAT:
    ihm_initCA(&ihm_CA[KIHM_CURVE_VBAT], VBAT_WIN_TITLE, "VBAT", 2, 1);
    strcpy(ihm_CA[KIHM_CURVE_VBAT].legend[0], "Temperature [K]");
    strcpy(ihm_CA[KIHM_CURVE_VBAT].legend[1], "Vbat [%]");
    ihm_CA[KIHM_CURVE_VBAT].GC[0] = ihm_GC[KIHM_GC_RED];
    ihm_CA[KIHM_CURVE_VBAT].GC[1] = ihm_GC[KIHM_GC_BLUE];
    scale_idx = 0;
    ihm_CA[KIHM_CURVE_VBAT].tSA[scale_idx].phys_orig =   0.0;
    ihm_CA[KIHM_CURVE_VBAT].tSA[scale_idx].phys_min  =   0.0;
    ihm_CA[KIHM_CURVE_VBAT].tSA[scale_idx].phys_max  =  105.0;
    ihm_CA[KIHM_CURVE_VBAT].tSA[scale_idx].phys_step =   10.0;
    strcpy(ihm_CA[KIHM_CURVE_VBAT].tSA[scale_idx].grad_format, "% 5.1f");
    strcpy(ihm_CA[KIHM_CURVE_VBAT].val_format , "% 7.2f");
    ihm_createCurve(&ihm_CA[KIHM_CURVE_VBAT]);
    printf("   ihm_init : Vbat display\n");
    break;

  /* PWM display */
  /* ----------- */
  case KIHM_CURVE_PWM:
    ihm_initCA(&ihm_CA[KIHM_CURVE_PWM], PWM_WIN_TITLE, "PWM", 4, 1);
    strcpy(ihm_CA[KIHM_CURVE_PWM].legend[0], "PWM M1");
    strcpy(ihm_CA[KIHM_CURVE_PWM].legend[1], "PWM M2");
    strcpy(ihm_CA[KIHM_CURVE_PWM].legend[2], "PWM M3");
    strcpy(ihm_CA[KIHM_CURVE_PWM].legend[3], "PWM M4");
    ihm_CA[KIHM_CURVE_PWM].GC[0] = ihm_GC[KIHM_GC_RED];
    ihm_CA[KIHM_CURVE_PWM].GC[1] = ihm_GC[KIHM_GC_GREEN];
    ihm_CA[KIHM_CURVE_PWM].GC[2] = ihm_GC[KIHM_GC_BLUE];
    ihm_CA[KIHM_CURVE_PWM].GC[3] = ihm_GC[KIHM_GC_BLACK];
    scale_idx = 0;
    ihm_CA[KIHM_CURVE_PWM].tSA[scale_idx].phys_orig =   0.0;
    ihm_CA[KIHM_CURVE_PWM].tSA[scale_idx].phys_min  =   0.0;
    ihm_CA[KIHM_CURVE_PWM].tSA[scale_idx].phys_max  = 256.0;
    ihm_CA[KIHM_CURVE_PWM].tSA[scale_idx].phys_step =  32.0;
    strcpy(ihm_CA[KIHM_CURVE_PWM].tSA[scale_idx].grad_format, "% 5.1f");
    strcpy(ihm_CA[KIHM_CURVE_PWM].val_format , "% 5.0f");
    ihm_createCurve(&ihm_CA[KIHM_CURVE_PWM]);
    printf("   ihm_init : PWM display\n");
    break;

  /* Altitude display */
  /* ----------- */
  case KIHM_CURVE_ALTITUDE:
    ihm_initCA(&ihm_CA[KIHM_CURVE_ALTITUDE], ALTITUDE_WIN_TITLE, "Altitude", 4,2);
    strcpy(ihm_CA[KIHM_CURVE_ALTITUDE].legend[0], "Measure");
    strcpy(ihm_CA[KIHM_CURVE_ALTITUDE].legend[1], "Vision");
    strcpy(ihm_CA[KIHM_CURVE_ALTITUDE].legend[2], "Reference");
    strcpy(ihm_CA[KIHM_CURVE_ALTITUDE].legend[3], "Raw");
    ihm_CA[KIHM_CURVE_ALTITUDE].GC[0] = ihm_GC[KIHM_GC_BLUE];
    ihm_CA[KIHM_CURVE_ALTITUDE].GC[1] = ihm_GC[KIHM_GC_GREEN];
    ihm_CA[KIHM_CURVE_ALTITUDE].GC[2] = ihm_GC[KIHM_GC_BLACK];
    ihm_CA[KIHM_CURVE_ALTITUDE].GC[3] = ihm_GC[KIHM_GC_RED];
    scale_idx = 0;
    ihm_CA[KIHM_CURVE_ALTITUDE].tSA[scale_idx].phys_orig =   0.0;
    ihm_CA[KIHM_CURVE_ALTITUDE].tSA[scale_idx].phys_min  =   0.0;
    ihm_CA[KIHM_CURVE_ALTITUDE].tSA[scale_idx].phys_max  = 4000.0;
    ihm_CA[KIHM_CURVE_ALTITUDE].tSA[scale_idx].phys_step =  1000.0;
    strcpy(ihm_CA[KIHM_CURVE_ALTITUDE].tSA[scale_idx].grad_format, "% 5.1f");
    strcpy(ihm_CA[KIHM_CURVE_ALTITUDE].val_format , "% 4.0f");
    scale_idx++;
    ihm_CA[KIHM_CURVE_ALTITUDE].tSA[scale_idx].phys_orig =   0.0;
    ihm_CA[KIHM_CURVE_ALTITUDE].tSA[scale_idx].phys_min  =   0.0;
    ihm_CA[KIHM_CURVE_ALTITUDE].tSA[scale_idx].phys_max  = 8000.0;
    ihm_CA[KIHM_CURVE_ALTITUDE].tSA[scale_idx].phys_step = 2000.0;
    strcpy(ihm_CA[KIHM_CURVE_ALTITUDE].tSA[scale_idx].grad_format, "% 5.1f");
    strcpy(ihm_CA[KIHM_CURVE_ALTITUDE].val_format , "% 4.0f");
    ihm_createCurve(&ihm_CA[KIHM_CURVE_ALTITUDE]);
    printf("   ihm_init : Altitude display\n");
    break;

  /* VX display */
  /* ----------- */
  case KIHM_CURVE_VX:
    ihm_initCA(&ihm_CA[KIHM_CURVE_VX], VX_WIN_TITLE, "VX", 3, 3);
    strcpy(ihm_CA[KIHM_CURVE_VX].legend[0], "VISION");
    strcpy(ihm_CA[KIHM_CURVE_VX].legend[1], "FUSION");
    strcpy(ihm_CA[KIHM_CURVE_VX].legend[2], "POLARIS");
    ihm_CA[KIHM_CURVE_VX].GC[0] = ihm_GC[KIHM_GC_RED];
    ihm_CA[KIHM_CURVE_VX].GC[1] = ihm_GC[KIHM_GC_BLUE];
    ihm_CA[KIHM_CURVE_VX].GC[2] = ihm_GC[KIHM_GC_GREEN];
    scale_idx = 0;
    ihm_CA[KIHM_CURVE_VX].tSA[scale_idx].phys_orig =   0.0;
    ihm_CA[KIHM_CURVE_VX].tSA[scale_idx].phys_min  =  -100;
    ihm_CA[KIHM_CURVE_VX].tSA[scale_idx].phys_max  =   100;
    ihm_CA[KIHM_CURVE_VX].tSA[scale_idx].phys_step =   50;
    strcpy(ihm_CA[KIHM_CURVE_VX].tSA[scale_idx].grad_format, "% 5.1f");
    strcpy(ihm_CA[KIHM_CURVE_VX].val_format , "% 6.4f");
    scale_idx++;
    ihm_CA[KIHM_CURVE_VX].tSA[scale_idx].phys_orig =   0.0;
    ihm_CA[KIHM_CURVE_VX].tSA[scale_idx].phys_min  =  -500;
    ihm_CA[KIHM_CURVE_VX].tSA[scale_idx].phys_max  =   500;
    ihm_CA[KIHM_CURVE_VX].tSA[scale_idx].phys_step =   250;
    strcpy(ihm_CA[KIHM_CURVE_VX].tSA[scale_idx].grad_format, "% 5.1f");
    strcpy(ihm_CA[KIHM_CURVE_VX].val_format , "% 6.4f");
    scale_idx++;
    ihm_CA[KIHM_CURVE_VX].tSA[scale_idx].phys_orig =   0.0;
    ihm_CA[KIHM_CURVE_VX].tSA[scale_idx].phys_min  =  -1000;
    ihm_CA[KIHM_CURVE_VX].tSA[scale_idx].phys_max  =   1000;
    ihm_CA[KIHM_CURVE_VX].tSA[scale_idx].phys_step =   500;
    strcpy(ihm_CA[KIHM_CURVE_VX].tSA[scale_idx].grad_format, "% 5.1f");
    strcpy(ihm_CA[KIHM_CURVE_VX].val_format , "% 6.4f");
    ihm_createCurve(&ihm_CA[KIHM_CURVE_VX]);
    printf("   ihm_init : VX display\n");
    break;

  /* VY display */
  /* ----------- */
  case KIHM_CURVE_VY:
    ihm_initCA(&ihm_CA[KIHM_CURVE_VY], VY_WIN_TITLE, "VY", 3, 3);
    strcpy(ihm_CA[KIHM_CURVE_VY].legend[0], "VISION");
    strcpy(ihm_CA[KIHM_CURVE_VY].legend[1], "FUSION");
    strcpy(ihm_CA[KIHM_CURVE_VY].legend[2], "POLARIS");
    ihm_CA[KIHM_CURVE_VY].GC[0] = ihm_GC[KIHM_GC_RED];
    ihm_CA[KIHM_CURVE_VY].GC[1] = ihm_GC[KIHM_GC_BLUE];
    ihm_CA[KIHM_CURVE_VY].GC[2] = ihm_GC[KIHM_GC_GREEN];
    scale_idx = 0;
    ihm_CA[KIHM_CURVE_VY].tSA[scale_idx].phys_orig =   0.0;
    ihm_CA[KIHM_CURVE_VY].tSA[scale_idx].phys_min  =  -100;
    ihm_CA[KIHM_CURVE_VY].tSA[scale_idx].phys_max  =   100;
    ihm_CA[KIHM_CURVE_VY].tSA[scale_idx].phys_step =   50;
    strcpy(ihm_CA[KIHM_CURVE_VY].tSA[scale_idx].grad_format, "% 5.1f");
    strcpy(ihm_CA[KIHM_CURVE_VY].val_format , "% 6.4f");
    scale_idx++;
    ihm_CA[KIHM_CURVE_VY].tSA[scale_idx].phys_orig =   0.0;
    ihm_CA[KIHM_CURVE_VY].tSA[scale_idx].phys_min  =  -500;
    ihm_CA[KIHM_CURVE_VY].tSA[scale_idx].phys_max  =   500;
    ihm_CA[KIHM_CURVE_VY].tSA[scale_idx].phys_step =   250;
    strcpy(ihm_CA[KIHM_CURVE_VY].tSA[scale_idx].grad_format, "% 5.1f");
    strcpy(ihm_CA[KIHM_CURVE_VY].val_format , "% 6.4f");
    scale_idx++;
    ihm_CA[KIHM_CURVE_VY].tSA[scale_idx].phys_orig =   0.0;
    ihm_CA[KIHM_CURVE_VY].tSA[scale_idx].phys_min  =  -1000;
    ihm_CA[KIHM_CURVE_VY].tSA[scale_idx].phys_max  =   1000;
    ihm_CA[KIHM_CURVE_VY].tSA[scale_idx].phys_step =   500;
    strcpy(ihm_CA[KIHM_CURVE_VY].tSA[scale_idx].grad_format, "% 5.1f");
    strcpy(ihm_CA[KIHM_CURVE_VY].val_format , "% 6.4f");
    ihm_createCurve(&ihm_CA[KIHM_CURVE_VY]);
    printf("   ihm_init : VY display\n");
    break;


  /* VZ display */
  /* ----------- */
  case KIHM_CURVE_VZ:
    ihm_initCA(&ihm_CA[KIHM_CURVE_VZ], VZ_WIN_TITLE, "VZ", 4, 3);
    strcpy(ihm_CA[KIHM_CURVE_VZ].legend[0], "VISION");
    strcpy(ihm_CA[KIHM_CURVE_VZ].legend[1], "FUSION");
    strcpy(ihm_CA[KIHM_CURVE_VZ].legend[2], "POLARIS");
    strcpy(ihm_CA[KIHM_CURVE_VZ].legend[3], "REF");
    ihm_CA[KIHM_CURVE_VZ].GC[0] = ihm_GC[KIHM_GC_RED];
    ihm_CA[KIHM_CURVE_VZ].GC[1] = ihm_GC[KIHM_GC_BLUE];
    ihm_CA[KIHM_CURVE_VZ].GC[2] = ihm_GC[KIHM_GC_GREEN];
    ihm_CA[KIHM_CURVE_VZ].GC[3] = ihm_GC[KIHM_GC_BLACK];
    scale_idx = 0;
    ihm_CA[KIHM_CURVE_VZ].tSA[scale_idx].phys_orig =   0.0;
    ihm_CA[KIHM_CURVE_VZ].tSA[scale_idx].phys_min  =  -100;
    ihm_CA[KIHM_CURVE_VZ].tSA[scale_idx].phys_max  =   100;
    ihm_CA[KIHM_CURVE_VZ].tSA[scale_idx].phys_step =   50;
    strcpy(ihm_CA[KIHM_CURVE_VZ].tSA[scale_idx].grad_format, "% 5.1f");
    strcpy(ihm_CA[KIHM_CURVE_VZ].val_format , "% 6.4f");
    scale_idx++;
    ihm_CA[KIHM_CURVE_VZ].tSA[scale_idx].phys_orig =   0.0;
    ihm_CA[KIHM_CURVE_VZ].tSA[scale_idx].phys_min  =  -500;
    ihm_CA[KIHM_CURVE_VZ].tSA[scale_idx].phys_max  =   500;
    ihm_CA[KIHM_CURVE_VZ].tSA[scale_idx].phys_step =   250;
    strcpy(ihm_CA[KIHM_CURVE_VZ].tSA[scale_idx].grad_format, "% 5.1f");
    strcpy(ihm_CA[KIHM_CURVE_VZ].val_format , "% 6.4f");
    scale_idx++;
    ihm_CA[KIHM_CURVE_VZ].tSA[scale_idx].phys_orig =   0.0;
    ihm_CA[KIHM_CURVE_VZ].tSA[scale_idx].phys_min  =  -1000;
    ihm_CA[KIHM_CURVE_VZ].tSA[scale_idx].phys_max  =   1000;
    ihm_CA[KIHM_CURVE_VZ].tSA[scale_idx].phys_step =   500;
    strcpy(ihm_CA[KIHM_CURVE_VZ].tSA[scale_idx].grad_format, "% 5.1f");
    strcpy(ihm_CA[KIHM_CURVE_VZ].val_format , "% 6.4f");
    ihm_createCurve(&ihm_CA[KIHM_CURVE_VZ]);
    printf("   ihm_init : VZ display\n");
    break;

  /* Motor current display */
  /* ----------- */
  case KIHM_CURVE_CURRENT:
    ihm_initCA(&ihm_CA[KIHM_CURVE_CURRENT], CURRENT_WIN_TITLE, "Current", 4, 1);
    strcpy(ihm_CA[KIHM_CURVE_CURRENT].legend[0], "Current M1");
    strcpy(ihm_CA[KIHM_CURVE_CURRENT].legend[1], "Current M2");
    strcpy(ihm_CA[KIHM_CURVE_CURRENT].legend[2], "Current M3");
    strcpy(ihm_CA[KIHM_CURVE_CURRENT].legend[3], "Current M4");
    ihm_CA[KIHM_CURVE_CURRENT].GC[0] = ihm_GC[KIHM_GC_RED];
    ihm_CA[KIHM_CURVE_CURRENT].GC[1] = ihm_GC[KIHM_GC_BLUE];
    ihm_CA[KIHM_CURVE_CURRENT].GC[2] = ihm_GC[KIHM_GC_GREEN];
    ihm_CA[KIHM_CURVE_CURRENT].GC[3] = ihm_GC[KIHM_GC_BLACK];
    scale_idx = 0;
    ihm_CA[KIHM_CURVE_CURRENT].tSA[scale_idx].phys_orig =   0.0;
    ihm_CA[KIHM_CURVE_CURRENT].tSA[scale_idx].phys_min  =   0;
    ihm_CA[KIHM_CURVE_CURRENT].tSA[scale_idx].phys_max  =   5000;
    ihm_CA[KIHM_CURVE_CURRENT].tSA[scale_idx].phys_step =   1000;
    strcpy(ihm_CA[KIHM_CURVE_CURRENT].tSA[scale_idx].grad_format, "% 5.1f");
    strcpy(ihm_CA[KIHM_CURVE_CURRENT].val_format , "% 6.4f");
    ihm_createCurve(&ihm_CA[KIHM_CURVE_CURRENT]);
    printf("   ihm_init : Current display\n");
    break;

  default:
    printf("Unable to initialize curve: Unknown curve\n");
    break;
  }
}

static void color_update( GtkComboBox *combo,
                          gpointer    data )
{
  gchar *color;
  gint pos;
  pos = gtk_combo_box_get_active( combo );
  color = gtk_combo_box_get_active_text ( combo );

  ardrone_at_set_enemy_colors( pos + 1 );

  DEBUG_PRINT_SDK ("Enter in color_update with color %s with idx = %d \n", color, pos+1);
}

static void vision_select( GtkComboBox *combo,
                           gpointer    data )
{

  gint pos;
  pos = gtk_combo_box_get_active( combo );
  DEBUG_PRINT_SDK( "Enter in vision_select with value : %d \n", pos );

  ardrone_at_cad( pos, 0);
}


/*Stephane
Callback function for the "Fly with Shell ?" combo box */
static void shell_select( GtkComboBox *combo,
                           gpointer    data )
{

  gint pos;
  pos = gtk_combo_box_get_active( combo );
  DEBUG_PRINT_SDK( "Enter in shell_select with value : %d \n", pos );

  ardrone_at_set_flight_without_shell( (pos==0)?TRUE:FALSE );
}

static void anim_select( GtkComboBox *combo,
                         gpointer    data )
{
  gint pos;
  pos = gtk_combo_box_get_active( combo );
  DEBUG_PRINT_SDK( "Enter in anim_select with value : %d \n", pos );

  ardrone_at_set_anim( pos, MAYDAY_TIMEOUT[pos]);
}


static void led_select( GtkComboBox *combo,
                         gpointer    data )
{

  gint pos;
  private_data_led *pLed;
  pLed = (private_data_led*)data;
  pos = gtk_combo_box_get_active( combo );
  if(pLed != NULL )
  {
     DEBUG_PRINT_SDK( "Enter in led_select with pos=%d, freq=%f, duration=%d\n", pos,
                                (float)gtk_spin_button_get_value( GTK_SPIN_BUTTON(pLed->freq) ),
                                gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(pLed->duration) ));

  ardrone_at_set_led_animation( pos,
                                (float)gtk_spin_button_get_value( GTK_SPIN_BUTTON(pLed->freq) ),
                                gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(pLed->duration) ));
  }
}

void ihm_init()
{
  int k;
  int ii;

  ihm_val_idx = 0;

  /* Main window */
  /* ----------- */
  GtkWidget *MainWin, *vBox;
  MainWin = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  //  gtk_window_set_position(GTK_WINDOW(MainWin), GTK_WIN_POS_CENTER_ALWAYS);
  //  gtk_window_set_default_size(GTK_WINDOW(MainWin), KIHM_WX, KIHM_WY);
  gtk_container_set_border_width(GTK_CONTAINER(MainWin), KIHM_BORDER);
  gtk_signal_connect( GTK_OBJECT(MainWin),
                      "destroy",
                      G_CALLBACK(ihm_main_quit),
                      NULL );
  gtk_window_set_title(GTK_WINDOW(MainWin), "Drone attitude viewer");
  gtk_widget_realize(MainWin);
  vBox = gtk_vbox_new(FALSE, 0);
  printf("   ihm_init : main window\n");

  // graphic contexts creation (MainWin must be realized)
  ihm_createGCs(MainWin);




  /* Control States interface */
  /* ------------------------ */

  // gtk_box_pack_start(GTK_BOX(vBox), hBox_CtrlState, FALSE, TRUE,  0);

  GtkWidget* frame_ctrl_mykonos = gtk_frame_new("Mykonos states");
  gtk_box_pack_start(GTK_BOX(vBox), frame_ctrl_mykonos, FALSE, FALSE, 0);

  GtkWidget* hBox_CtrlState = gtk_hbox_new(FALSE, 20);
  gtk_container_add( GTK_CONTAINER(frame_ctrl_mykonos), hBox_CtrlState );

  GtkWidget* label_mykonos_states[NB_ARDRONE_STATES];
  label_mykonos_states[0] = gtk_label_new("Mykonos Global State: ");
  label_mykonos_states[1] = gtk_label_new("Mykonos Control State: ");
  label_mykonos_states[2] = gtk_label_new("Mykonos Vision State: ");

  vp_os_memset(label_mykonos_state_value, 0, sizeof(label_mykonos_state_value));
  vp_os_memset(label_ctrl_state_value, 0, sizeof(label_ctrl_state_value));
  vp_os_memset(label_vision_state_value, 0, sizeof(label_vision_state_value));

  strcat(label_mykonos_state_value, "Not Connected");
  strcat(label_ctrl_state_value, "Not Connected");
  strcat(label_vision_state_value, "Not Connected");

  label_mykonos_values[0] = gtk_label_new(label_mykonos_state_value);
  label_mykonos_values[1] = gtk_label_new(label_ctrl_state_value);
  label_mykonos_values[2] = gtk_label_new(label_vision_state_value);

  // Start button indicating flight or land state
  label_start_button_state = gtk_label_new("Start button: ");
  darea_start_button_state = gtk_drawing_area_new();
  gtk_drawing_area_size((GtkDrawingArea*)darea_start_button_state, START_BUTTON_DA_SIZE, START_BUTTON_DA_SIZE );

  gtk_box_pack_start(GTK_BOX(hBox_CtrlState), label_start_button_state, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hBox_CtrlState), darea_start_button_state, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hBox_CtrlState), label_mykonos_states[0], FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hBox_CtrlState), label_mykonos_values[0], FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hBox_CtrlState), label_mykonos_states[1], FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hBox_CtrlState), label_mykonos_values[1], FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hBox_CtrlState), label_mykonos_states[2], FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hBox_CtrlState), label_mykonos_values[2], FALSE, FALSE, 0);

  /* Main control interface */
  /* ---------------------- */
  GtkWidget* frame_AngularRateCtrlGain;
  GtkWidget* frame_AttitudeCtrlGain;
  GtkWidget* frame_MiscellaneousVar;
  GtkWidget* frame_AltitudeCtrlGain;
  GtkWidget* frame_FixPointCtrlGain;
  GtkWidget* button_SendCtrlGains, *button_SendFlatTrims, *button_SetConfigParams;
  GtkWidget* label_AngularRateCtrlGain[NB_GAINS_W];
  GtkWidget* label_AttitudeCtrlGain[NB_GAINS_EA];
  GtkWidget* label_MiscellaneousVar[NB_MISC_VARS];
  GtkWidget* label_AltitudeCtrlGain[NB_GAINS_ALT];
  GtkWidget* label_FixPointCtrlGain[NB_GAINS_FP];
  GtkWidget* entry_AngularRateCtrlGain[NB_GAINS_W];
  GtkWidget* entry_AttitudeCtrlGain[NB_GAINS_EA];
  GtkWidget* entry_MiscellaneousVar[NB_MISC_VARS];
  GtkWidget* entry_AltitudeCtrlGain[NB_GAINS_ALT];
  GtkWidget* entry_FixPointCtrlGain[NB_GAINS_FP];
  GtkWidget* hBox_CtrlGain;
  GtkWidget* hBox_AltCtrlGain;
  GtkWidget* hBox_AngularRateCtrlGain;
  GtkWidget* hBox_AttitudeCtrlGain;
  GtkWidget* hBox_AltitudeCtrlGain;
  GtkWidget* hBox_FixPointCtrlGain;
  GtkWidget* hBox_MiscellaneousVar;
  GtkWidget* ctrl_gain_h_separator;

   // Frames
  frame_AngularRateCtrlGain = gtk_frame_new("Angular rate control gain");
  frame_AttitudeCtrlGain    = gtk_frame_new("Attitude control gains");
  frame_AltitudeCtrlGain    = gtk_frame_new("Altitude control gains");
  frame_FixPointCtrlGain    = gtk_frame_new("Fixed Point control gains");
  frame_MiscellaneousVar    = gtk_frame_new("MiscVars");
  printf("   ihm_init : Frames Control gains interface\n");

  // Labels
  label_AngularRateCtrlGain[0] = gtk_label_new("PQ_Kp");
  label_AngularRateCtrlGain[1] = gtk_label_new("  R_Kp");
  label_AngularRateCtrlGain[2] = gtk_label_new("  R_Ki");

  label_AttitudeCtrlGain[0] = gtk_label_new("  EA_Kp");
  label_AttitudeCtrlGain[1] = gtk_label_new("  EA_Ki");

  label_AltitudeCtrlGain[0] = gtk_label_new(" A_Kp");
  label_AltitudeCtrlGain[1] = gtk_label_new(" A_Ki");
  label_AltitudeCtrlGain[2] = gtk_label_new(" Vz_Kp");
  label_AltitudeCtrlGain[3] = gtk_label_new(" VZ_Ki");

  label_FixPointCtrlGain[0] = gtk_label_new(" H_Kp");
  label_FixPointCtrlGain[1] = gtk_label_new(" H_Ki");

  label_MiscellaneousVar[0] = gtk_label_new("MV0");
  label_MiscellaneousVar[1] = gtk_label_new("MV1");
  label_MiscellaneousVar[2] = gtk_label_new("MV2");
  label_MiscellaneousVar[3] = gtk_label_new("MV3");

  // Fill entries with default values
  if (get_flight_without_shell())
  {// Shelless
    printf("   ihm_init : Labels Control gains interface (Shelless)\n");
    tab_g[0] = CTRL_DEFAULT_NUM_PQ_KP_NO_SHELL;
    tab_g[1] = CTRL_DEFAULT_NUM_R_KP;
    tab_g[2] = CTRL_DEFAULT_NUM_R_KI;
    tab_g[3] = CTRL_DEFAULT_NUM_EA_KP_NO_SHELL;
    tab_g[4] = CTRL_DEFAULT_NUM_EA_KI_NO_SHELL;

    tab_ag[KIHM_AKP] = CTRL_DEFAULT_NUM_ALT_KP;
    tab_ag[KIHM_AKI] = CTRL_DEFAULT_NUM_ALT_KI;
    tab_ag[KIHM_AKD] = CTRL_DEFAULT_NUM_VZ_KP;
    tab_ag[KIHM_ATD] = CTRL_DEFAULT_NUM_VZ_KI;

    tab_fp_g[KIHM_FPKP] = CTRL_DEFAULT_NUM_HOVER_KP_NO_SHELL;
    tab_fp_g[KIHM_FPKI] = CTRL_DEFAULT_NUM_HOVER_KI_NO_SHELL;
  }
  else
  {// With shell
  printf("   ihm_init : Labels Control gains interface (with Shell)\n");
    tab_g[0] = CTRL_DEFAULT_NUM_PQ_KP_SHELL;
    tab_g[1] = CTRL_DEFAULT_NUM_R_KP;
    tab_g[2] = CTRL_DEFAULT_NUM_R_KI;
    tab_g[3] = CTRL_DEFAULT_NUM_EA_KP_SHELL;
    tab_g[4] = CTRL_DEFAULT_NUM_EA_KI_SHELL;

    tab_ag[KIHM_AKP] = CTRL_DEFAULT_NUM_ALT_KP;
    tab_ag[KIHM_AKI] = CTRL_DEFAULT_NUM_ALT_KI;
    tab_ag[KIHM_AKD] = CTRL_DEFAULT_NUM_VZ_KP;
    tab_ag[KIHM_ATD] = CTRL_DEFAULT_NUM_VZ_KI;

    tab_fp_g[KIHM_FPKP] = CTRL_DEFAULT_NUM_HOVER_KP_SHELL;
    tab_fp_g[KIHM_FPKI] = CTRL_DEFAULT_NUM_HOVER_KI_SHELL;
  }

  char label_gain[10];

  // Entries
  for (ii=0; ii<NB_GAINS_W; ii++) {
    entry_AngularRateCtrlGain[ii] = gtk_entry_new();
    gtk_widget_set_size_request(entry_AngularRateCtrlGain[ii], 60, 20);
    sprintf(label_gain, "%d", tab_g[ii]);
    gtk_entry_set_text( GTK_ENTRY(entry_AngularRateCtrlGain[ii]), label_gain);
  }
  for (ii=0; ii<NB_GAINS_EA; ii++) {
    entry_AttitudeCtrlGain[ii] = gtk_entry_new();
    gtk_widget_set_size_request(entry_AttitudeCtrlGain[ii], 60, 20);
    sprintf(label_gain, "%d", tab_g[NB_GAINS_W+ii]);
    gtk_entry_set_text( GTK_ENTRY(entry_AttitudeCtrlGain[ii]), label_gain);
  }
  for (ii=0; ii<NB_GAINS_ALT; ii++) {
    entry_AltitudeCtrlGain[ii] = gtk_entry_new();
    gtk_widget_set_size_request(entry_AltitudeCtrlGain[ii], 60, 20);
    sprintf(label_gain, "%d", tab_ag[ii]);
    gtk_entry_set_text( GTK_ENTRY(entry_AltitudeCtrlGain[ii]), label_gain);
  }
  for (ii=0; ii<NB_GAINS_FP; ii++) {
    entry_FixPointCtrlGain[ii] = gtk_entry_new();
    gtk_widget_set_size_request(entry_FixPointCtrlGain[ii], 60, 20);
    sprintf(label_gain, "%d", tab_fp_g[ii]);
    gtk_entry_set_text( GTK_ENTRY(entry_FixPointCtrlGain[ii]), label_gain);
  }
  for (ii=0; ii<NB_MISC_VARS; ii++) {
    entry_MiscellaneousVar[ii] = gtk_entry_new();
    gtk_widget_set_size_request(entry_MiscellaneousVar[ii], 60, 20);
    sprintf(label_gain, "%d", MiscVar[ii] );
    gtk_entry_set_text( GTK_ENTRY(entry_MiscellaneousVar[ii]), label_gain);
  }

  //  send_gains(pcfg); // send initial gain values
  for (ii=0; ii<NB_GAINS_W; ii++) {
    entry_PID[ii] = entry_AngularRateCtrlGain[ii];
  }
  for (ii=0; ii<NB_GAINS_EA; ii++) {
    entry_PID[NB_GAINS_W+ii] = entry_AttitudeCtrlGain[ii];
  }
  for (ii=0; ii<NB_GAINS_ALT; ii++) {
    entry_PID[NB_GAINS+ii] = entry_AltitudeCtrlGain[ii];
  }
  for (ii=0; ii<NB_GAINS_FP; ii++) {
    entry_PID[NB_GAINS+NB_GAINS_ALT+ii] = entry_FixPointCtrlGain[ii];
  }
  for (ii=0; ii<NB_MISC_VARS; ii++) {
    entry_MiscVar[ii] = entry_MiscellaneousVar[ii];
  }

  printf("   ihm_init : Entries Control gains interface\n");

  // Boxes
  hBox_CtrlGain            = gtk_hbox_new(FALSE, 20);
  hBox_AltCtrlGain         = gtk_hbox_new(FALSE, 20);
  hBox_AngularRateCtrlGain = gtk_hbox_new(FALSE, 10);
  hBox_AttitudeCtrlGain    = gtk_hbox_new(FALSE, 10);
  hBox_AltitudeCtrlGain    = gtk_hbox_new(FALSE, 10);
  hBox_FixPointCtrlGain    = gtk_hbox_new(FALSE, 10);
  hBox_MiscellaneousVar    = gtk_hbox_new(FALSE, 20);
  printf("   ihm_init : Boxes Control gains interface\n");

  // Separators
  ctrl_gain_h_separator = gtk_hseparator_new();

  // Buttons
   button_SendCtrlGains = gtk_button_new_with_label("Send control gains");
  GList* children_list = gtk_container_get_children(GTK_CONTAINER(button_SendCtrlGains));
  gtk_label_set_justify(GTK_LABEL(children_list->data), GTK_JUSTIFY_CENTER);
  /* Connect the "clicked" signal of the button to our callback */
  g_signal_connect(G_OBJECT(button_SendCtrlGains), "clicked", G_CALLBACK(ihm_sendGains), (gpointer)NULL);




  printf("   ihm_init : Button Control gains interface\n");

  button_SetConfigParams = gtk_button_new_with_label("Set configuration\nparams");
  g_signal_connect(G_OBJECT(button_SetConfigParams), "clicked", G_CALLBACK(ihm_setConfigParamsWindow), (gpointer)NULL);

  // Widget hierarchy construction
  for (ii=0; ii<NB_GAINS_W; ii++) {
    gtk_box_pack_start(GTK_BOX(hBox_AngularRateCtrlGain), label_AngularRateCtrlGain[ii], FALSE , FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hBox_AngularRateCtrlGain), entry_AngularRateCtrlGain[ii], FALSE , FALSE, 0);
  }
  gtk_container_add(GTK_CONTAINER(frame_AngularRateCtrlGain), hBox_AngularRateCtrlGain);

  for (ii=0; ii<NB_MISC_VARS; ii++) {
    gtk_box_pack_start(GTK_BOX(hBox_MiscellaneousVar), label_MiscellaneousVar[ii], FALSE , FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hBox_MiscellaneousVar), entry_MiscellaneousVar[ii], FALSE , FALSE, 0);
  }
  gtk_container_add(GTK_CONTAINER(frame_MiscellaneousVar), hBox_MiscellaneousVar);

  for (ii=0; ii<NB_GAINS_ALT; ii++) {
    gtk_box_pack_start(GTK_BOX(hBox_AltitudeCtrlGain), label_AltitudeCtrlGain[ii], FALSE , FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hBox_AltitudeCtrlGain),  entry_AltitudeCtrlGain[ii], FALSE , FALSE, 0);
  }
  gtk_container_add(GTK_CONTAINER(frame_AltitudeCtrlGain), hBox_AltitudeCtrlGain);

  for (ii=0; ii<NB_GAINS_FP; ii++) {
    gtk_box_pack_start(GTK_BOX(hBox_FixPointCtrlGain), label_FixPointCtrlGain[ii], FALSE , FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hBox_FixPointCtrlGain),  entry_FixPointCtrlGain[ii], FALSE , FALSE, 0);
  }
  gtk_container_add(GTK_CONTAINER(frame_FixPointCtrlGain), hBox_FixPointCtrlGain);

  for (ii=0; ii<NB_GAINS_EA; ii++) {
    gtk_box_pack_start(GTK_BOX(hBox_AttitudeCtrlGain), label_AttitudeCtrlGain[ii], FALSE , FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hBox_AttitudeCtrlGain),  entry_AttitudeCtrlGain[ii], FALSE , FALSE, 0);
  }
  gtk_container_add(GTK_CONTAINER(frame_AttitudeCtrlGain), hBox_AttitudeCtrlGain);

  gtk_box_pack_start(GTK_BOX(hBox_CtrlGain), frame_AngularRateCtrlGain, FALSE , FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hBox_CtrlGain), frame_AttitudeCtrlGain   , FALSE , FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hBox_CtrlGain), frame_FixPointCtrlGain , FALSE , FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hBox_CtrlGain), button_SendCtrlGains     , FALSE , FALSE, 0);

  gtk_box_pack_start(GTK_BOX(hBox_AltCtrlGain), frame_MiscellaneousVar , FALSE , FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hBox_AltCtrlGain), frame_AltitudeCtrlGain , FALSE , FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hBox_AltCtrlGain), button_SetConfigParams , FALSE , FALSE, 0);
  printf("   ihm_init : Control gains interface\n");

  /* Attitude display */
  /* ---------------- */
  GtkWidget *frame_Attitude, *hBox_Attitude;
  frame_Attitude = gtk_frame_new("Attitude (Euler angles)");
  hBox_Attitude  = gtk_hbox_new(FALSE, 0);
  ihm_DA_att[KIHM_DA_ATT_PITCH] = gtk_drawing_area_new();
  gtk_drawing_area_size((GtkDrawingArea*)ihm_DA_att[KIHM_DA_ATT_PITCH], KIHM_DAREA_ANGLE_X_SIZE, KIHM_DAREA_ANGLE_Y_SIZE);
  ihm_DA_att[KIHM_DA_ATT_ROLL ] = gtk_drawing_area_new();
  gtk_drawing_area_size((GtkDrawingArea*)ihm_DA_att[KIHM_DA_ATT_ROLL ], KIHM_DAREA_ANGLE_X_SIZE, KIHM_DAREA_ANGLE_Y_SIZE);
  ihm_DA_att[KIHM_DA_ATT_YAW  ] = gtk_drawing_area_new();
  gtk_drawing_area_size((GtkDrawingArea*)ihm_DA_att[KIHM_DA_ATT_YAW  ], KIHM_DAREA_ANGLE_X_SIZE, KIHM_DAREA_ANGLE_Y_SIZE);
  ihm_DA_att[KIHM_DA_ATT_DIR  ] = gtk_drawing_area_new();
  gtk_drawing_area_size((GtkDrawingArea*)ihm_DA_att[KIHM_DA_ATT_DIR  ], KIHM_DAREA_ANGLE_X_SIZE, KIHM_DAREA_ANGLE_Y_SIZE);

  // Widget hierarchy construction
  gtk_box_pack_start(GTK_BOX(hBox_Attitude), ihm_DA_att[KIHM_DA_ATT_PITCH], TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hBox_Attitude), ihm_DA_att[KIHM_DA_ATT_ROLL ], TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hBox_Attitude), ihm_DA_att[KIHM_DA_ATT_YAW  ], TRUE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hBox_Attitude), ihm_DA_att[KIHM_DA_ATT_DIR  ], TRUE, FALSE, 0);
  gtk_container_add(GTK_CONTAINER(frame_Attitude), hBox_Attitude);
  printf("   ihm_init : Attitude display\n");

  // PangoLayouts
  ihm_PL_DAtheta = pango_layout_new(gtk_widget_get_pango_context(ihm_DA_att[KIHM_DA_ATT_PITCH]));
  ihm_PL_DAphi   = pango_layout_new(gtk_widget_get_pango_context(ihm_DA_att[KIHM_DA_ATT_ROLL ]));
  ihm_PL_DApsi   = pango_layout_new(gtk_widget_get_pango_context(ihm_DA_att[KIHM_DA_ATT_YAW  ]));
  ihm_PL_DAdir   = pango_layout_new(gtk_widget_get_pango_context(ihm_DA_att[KIHM_DA_ATT_DIR  ]));

  /* RadioCommand references display */
  /* ------------------------------- */
  GtkWidget *frame_RCref;
  GtkWidget * box_RCref;

  frame_RCref = gtk_frame_new("Euler angles references");
  box_RCref = gtk_hbox_new(FALSE, 0);

  ihm_label_RCref = gtk_label_new("");
  PangoFontDescription* pPFD_RCref = pango_font_description_from_string("terminal 12"); // select a standard fixed font
  gtk_widget_modify_font(ihm_label_RCref, pPFD_RCref);
  gtk_box_pack_start(GTK_BOX(box_RCref),ihm_label_RCref, FALSE , FALSE, 0);

  button_SendFlatTrims = gtk_button_new_with_label("Flat trim");
  g_signal_connect(G_OBJECT(button_SendFlatTrims), "clicked", G_CALLBACK(ihm_sendFlatTrim), (gpointer)NULL);
  gtk_box_pack_start(GTK_BOX(box_RCref), button_SendFlatTrims     , FALSE , FALSE, 0);

  gtk_container_add(GTK_CONTAINER(frame_RCref), box_RCref);


  printf("   ihm_init : RadioCommand references display\n");

  /* Elapsed time frame */
  GtkWidget *frame_elapsedT, *hBox_RCref_ETime;
  frame_elapsedT = gtk_frame_new("Time (min:sec)");
  hBox_RCref_ETime = gtk_hbox_new(FALSE, 0);
  label_elapsedT = gtk_label_new("");
  gtk_container_add(GTK_CONTAINER(frame_elapsedT), label_elapsedT);
  gtk_box_pack_start(GTK_BOX(hBox_RCref_ETime), frame_RCref   , TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hBox_RCref_ETime), frame_elapsedT, TRUE, TRUE, 0);

  /* Curves creation and initialization */
  /* ---------------------------------- */
  for (k=0; k<KIHM_NB_CURVES; k++) {
    ihm_initCurves( ihm_CA[k].PrivateWin, k);
    windows_status[k] = WINDOW_OPENED;
  }

  /* Curves control display */
  /* ---------------------- */
  GtkWidget *button_FreezeCurves, *hBox_CurveCtrl[3], *vBox_CurveCtrl ;
  GtkWidget *pframe_CurveCtrl;

  /* Image display show button */
  /* ------------------------- */
  GtkWidget *label_to_fill_space;
  char show_image_ctrl_lbl[32];

  char *ctrl_lbl[KIHM_NB_CURVES] = {"Accelerations        ",
                                    "Angular rates        ",
                                    "Theta                ",
                                    "Phi                  ",
                                    "Vbat                 ",
                                    "PWM                  ",
                                    "Altitude             ",
                                    "Vx                   ",
                                    "Vy                   ",
                                    "Vz                   ",
                                    "Current              "};

  button_FreezeCurves = gtk_check_button_new_with_label("FREEZE CURVES");
  ihm_freeze_curves = FALSE;
  /* Connect the "clicked" signal of the button to our callback */
  g_signal_connect(G_OBJECT(button_FreezeCurves), "clicked", G_CALLBACK(ihm_freezeCurves), (gpointer)NULL);

  strncpy( show_image_ctrl_lbl, "VISION image", sizeof(show_image_ctrl_lbl) );
  button_show_image = gtk_check_button_new_with_label( show_image_ctrl_lbl );

  hBox_CurveCtrl[0] = gtk_hbox_new(FALSE, 0);
  hBox_CurveCtrl[1] = gtk_hbox_new(FALSE, 0);
  hBox_CurveCtrl[2] = gtk_hbox_new(FALSE, 0);
  vBox_CurveCtrl    = gtk_vbox_new(TRUE , 0);
  for (k=0; k<KIHM_NB_CURVES; k++) {
    strncpy(ihm_CA[k].ctrl_lbl, ctrl_lbl[k], sizeof(ihm_CA[k].ctrl_lbl));
    ihm_CA[k].ctrl_lbl[sizeof(ihm_CA[k].ctrl_lbl)-1] = 0;

    button_ShowCurve[k] = gtk_check_button_new_with_label(ihm_CA[k].ctrl_lbl);
		 if(k<6) gtk_box_pack_start(GTK_BOX(hBox_CurveCtrl[0]), button_ShowCurve[k], FALSE , FALSE, 0);
		 else gtk_box_pack_start(GTK_BOX(hBox_CurveCtrl[1]), button_ShowCurve[k], FALSE , FALSE, 0);
		 g_signal_connect(G_OBJECT(button_ShowCurve[k]), "clicked", G_CALLBACK(ihm_showCurves), (gpointer)k);
		 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button_ShowCurve[k]), FALSE );
	  }

	  label_to_fill_space = gtk_label_new("                                      ");

	  gtk_box_pack_start(GTK_BOX(vBox_CurveCtrl), hBox_CurveCtrl[0], FALSE , FALSE, 0);
	  gtk_box_pack_start(GTK_BOX(vBox_CurveCtrl), hBox_CurveCtrl[1], FALSE , FALSE, 0);
	  gtk_box_pack_start(GTK_BOX(vBox_CurveCtrl), hBox_CurveCtrl[2], FALSE , FALSE, 0);

	  pframe_CurveCtrl = gtk_frame_new("Curves control");

	  gtk_box_pack_start(GTK_BOX(hBox_CurveCtrl[1]), button_show_image  , FALSE , FALSE, 0);
	  gtk_box_pack_start(GTK_BOX(hBox_CurveCtrl[1]), label_to_fill_space, FALSE , FALSE, 0); // To fill space
	  gtk_box_pack_start(GTK_BOX(hBox_CurveCtrl[1]), button_FreezeCurves, FALSE , FALSE, 0);

	  gtk_container_add(GTK_CONTAINER(pframe_CurveCtrl), vBox_CurveCtrl);

	  /* Logs */
	  /* ---- */

	  GtkWidget* frame_logs_mykonos = gtk_frame_new("Logs");
	  GtkWidget* hBox_logs = gtk_hbox_new(FALSE, 20);
	  gtk_container_add( GTK_CONTAINER(frame_logs_mykonos), hBox_logs );

	  GtkWidget* frame_cad = gtk_frame_new("Camera detect");
	  GtkWidget* vBox_cad = gtk_hbox_new(FALSE, 20);
	  GtkWidget* hBox_cad = gtk_hbox_new(FALSE, 20);
	  GtkWidget* hBox_cad2 = gtk_hbox_new(FALSE, 20);



	  {
		  //GtkCellRenderer *cell;
		  GtkWidget* combo,*combo2;
		  GtkWidget* label,*label2;
        private_data_led *ldata;

        ldata = vp_os_malloc(sizeof(private_data_led));

		  label = gtk_label_new( "Change enemy color:" );
		  gtk_box_pack_start(GTK_BOX(hBox_cad), label, FALSE, FALSE, 0);

		  combo = gtk_combo_box_new_text();
		  g_signal_connect( G_OBJECT( combo ), "changed",
				  G_CALLBACK( color_update ), NULL );
		  //value start 0 => ORANGE_GREEN - 1
		  gtk_combo_box_insert_text( (GtkComboBox*)combo, 0, (const gchar*)"orange/green/orange");
		  gtk_combo_box_insert_text( (GtkComboBox*)combo, 1, (const gchar*)"orange/yellow/orange");
		  gtk_combo_box_insert_text( (GtkComboBox*)combo, 2, (const gchar*)"orange/blue/orange");

		  gtk_box_pack_start(GTK_BOX(hBox_cad), combo, FALSE, FALSE, 0);
		  gtk_widget_show( combo );

          label = gtk_label_new( "Select camera detect:" );
		  gtk_box_pack_start(GTK_BOX(hBox_cad), label, FALSE, FALSE, 0);
		  combo = gtk_combo_box_new_text();
		  g_signal_connect( G_OBJECT( combo ), "changed",
				  G_CALLBACK( vision_select ), NULL );

		  gtk_combo_box_insert_text( (GtkComboBox*)combo, 0, (const gchar*)"Camera horizontal");
		  gtk_combo_box_insert_text( (GtkComboBox*)combo, 1, (const gchar*)"Camera vertical");
		  gtk_combo_box_insert_text( (GtkComboBox*)combo, 2, (const gchar*)"Shell");
		  gtk_combo_box_insert_text( (GtkComboBox*)combo, 3, (const gchar*)"None");

		  gtk_box_pack_start(GTK_BOX(hBox_cad), combo, FALSE, FALSE, 0);


		  gtk_widget_show( combo );




    /* Stephane */
    /* Flight without shell option selection box */
        label2 = gtk_label_new( "Flight with shell ?" );
		  gtk_box_pack_start(GTK_BOX(hBox_cad2), label2, FALSE, FALSE, 0);
		  combo2 = gtk_combo_box_new_text();
		  g_signal_connect( G_OBJECT( combo2 ), "changed",
				  G_CALLBACK( shell_select ), NULL );
		  gtk_combo_box_insert_text( (GtkComboBox*)combo2, 0, (const gchar*)"No (tag detection searches 1 cockpit)");
		  gtk_combo_box_insert_text( (GtkComboBox*)combo2, 1, (const gchar*)"Yes (tag detection seacheds 2 tags)");

		  gtk_box_pack_start(GTK_BOX(hBox_cad2), combo2, FALSE, FALSE, 0);
          gtk_widget_show( combo2 );

    /* Shows the number of detected tags */
            nbDetectedTags_label = gtk_label_new((gchar*)"Number of detected tags : uninitialised data.");
            gtk_box_pack_start(GTK_BOX(hBox_cad2), nbDetectedTags_label, FALSE, FALSE, 0);
            gtk_container_add(GTK_CONTAINER(frame_cad), nbDetectedTags_label);

    /* Shows the histoy of detected tags */
            detectionHistory_label = gtk_label_new((gchar*)"Detected tags in the last second : ?");
            //gtk_box_pack_start(GTK_BOX(hBox_cad), detectionHistory_label, FALSE, FALSE, 0);
            //gtk_container_add(GTK_CONTAINER(frame_cad), detectionHistory_label);

    /* Builds the whole frame */

            gtk_box_pack_start(GTK_BOX(vBox_cad), hBox_cad, FALSE , FALSE, 0);
            gtk_box_pack_start(GTK_BOX(vBox_cad), hBox_cad2, FALSE , FALSE, 0);

            gtk_container_add( GTK_CONTAINER(frame_cad), vBox_cad );

	  }


      GtkWidget* frame_ledanim = gtk_frame_new("Led animations");
      GtkWidget* hBox_ledanim = gtk_hbox_new(FALSE, 20);
	  gtk_container_add( GTK_CONTAINER(frame_ledanim), hBox_ledanim );

     {
		  GtkWidget* combo;
		  GtkWidget* label;

         private_data_led *ldata;
         ldata = vp_os_malloc(sizeof(private_data_led));


         label = gtk_label_new( "Select anim:" );
		  gtk_box_pack_start(GTK_BOX(hBox_ledanim), label, FALSE, FALSE, 0);
		  combo = gtk_combo_box_new_text();
		  g_signal_connect( G_OBJECT( combo ), "changed",
				  G_CALLBACK( anim_select ), NULL );

		  gtk_combo_box_insert_text( (GtkComboBox*)combo, 0, (const gchar*)"Anim 0");
		  gtk_combo_box_insert_text( (GtkComboBox*)combo, 1, (const gchar*)"Anim 1");
		  gtk_combo_box_insert_text( (GtkComboBox*)combo, 2, (const gchar*)"Anim 2");
		  gtk_combo_box_insert_text( (GtkComboBox*)combo, 3, (const gchar*)"Anim 3");
		  gtk_combo_box_insert_text( (GtkComboBox*)combo, 4, (const gchar*)"Anim 4");
		  gtk_combo_box_insert_text( (GtkComboBox*)combo, 5, (const gchar*)"Anim 5");
		  gtk_combo_box_insert_text( (GtkComboBox*)combo, 6, (const gchar*)"Anim 6");
		  gtk_combo_box_insert_text( (GtkComboBox*)combo, 7, (const gchar*)"Anim 7");
		  gtk_combo_box_insert_text( (GtkComboBox*)combo, 8, (const gchar*)"Anim 8");

		  gtk_box_pack_start(GTK_BOX(hBox_ledanim), combo, FALSE, FALSE, 0);
		  gtk_widget_show( combo );

        label = gtk_label_new( "Select led:" );
		  gtk_box_pack_start(GTK_BOX(hBox_ledanim), label, FALSE, FALSE, 0);
		  combo = gtk_combo_box_new_text();
        g_signal_connect( G_OBJECT( combo ), "changed",
				  G_CALLBACK( led_select ), ldata );
		  gtk_combo_box_insert_text( (GtkComboBox*)combo, 0, (const gchar*)"BLINK_GREEN_RED");
		  gtk_combo_box_insert_text( (GtkComboBox*)combo, 1, (const gchar*)"BLINK_GREEN");
		  gtk_combo_box_insert_text( (GtkComboBox*)combo, 2, (const gchar*)"BLINK_RED");
		  gtk_combo_box_insert_text( (GtkComboBox*)combo, 3, (const gchar*)"SNAKE_GREEN_RED");
		  gtk_combo_box_insert_text( (GtkComboBox*)combo, 4, (const gchar*)"FIRE");
		  gtk_combo_box_insert_text( (GtkComboBox*)combo, 5, (const gchar*)"STANDARD");
		  gtk_combo_box_insert_text( (GtkComboBox*)combo, 6, (const gchar*)"RED");
		  gtk_combo_box_insert_text( (GtkComboBox*)combo, 7, (const gchar*)"GREEN");
		  gtk_combo_box_insert_text( (GtkComboBox*)combo, 8, (const gchar*)"RED_SNAKE");


		  gtk_box_pack_start(GTK_BOX(hBox_ledanim), combo, FALSE, FALSE, 0);
		  gtk_widget_show( combo );

        label = gtk_label_new( "Duration:" );
		  gtk_box_pack_start(GTK_BOX(hBox_ledanim), label, FALSE, FALSE, 0);
        ldata->duration = gtk_spin_button_new_with_range( 0, 10, 1);
		  gtk_box_pack_start(GTK_BOX(hBox_ledanim), ldata->duration, FALSE, FALSE, 0);
		  gtk_widget_show( ldata->duration );

        label = gtk_label_new( "Frequence:" );
		  gtk_box_pack_start(GTK_BOX(hBox_ledanim), label, FALSE, FALSE, 0);
        ldata->freq = gtk_spin_button_new_with_range( 0.1, 10.0, 0.1);
		  gtk_box_pack_start(GTK_BOX(hBox_ledanim), ldata->freq, FALSE, FALSE, 0);
		  gtk_widget_show( ldata->freq );
     }


     /* Image display main window */
	  /* ------------------------- */
	  create_image_window( );

	  /* vBox (main box) construction */
	  /* ---------------------------- */
	  gtk_box_pack_start(GTK_BOX(vBox), hBox_CtrlGain        , FALSE, TRUE, 0 );
	  gtk_box_pack_start(GTK_BOX(vBox), ctrl_gain_h_separator, FALSE, TRUE, 0 );
	  gtk_box_pack_start(GTK_BOX(vBox), hBox_AltCtrlGain     , FALSE, TRUE, 0 );
	  gtk_box_pack_start(GTK_BOX(vBox), frame_Attitude       , FALSE, TRUE, 5 );
	  gtk_box_pack_start(GTK_BOX(vBox), hBox_RCref_ETime     , FALSE, TRUE, 5 );
	  gtk_box_pack_start(GTK_BOX(vBox), pframe_CurveCtrl     , FALSE, TRUE, 5 );
	  gtk_box_pack_start(GTK_BOX(vBox), frame_logs_mykonos   , FALSE, TRUE, 0 );
	  gtk_box_pack_start(GTK_BOX(vBox), frame_cad            , FALSE, TRUE, 0 );

	  /*Stephane*/ gtk_box_pack_start(GTK_BOX(vBox), detectionHistory_label, FALSE, TRUE, 0 );
        gtk_box_pack_start(GTK_BOX(vBox), frame_ledanim        , FALSE, TRUE, 0 );


      /*Stephane*/ gtk_widget_show( detectionHistory_label );

	  gtk_container_add(GTK_CONTAINER(MainWin), vBox);
	  gtk_widget_show_all(MainWin);

	  printf("   ihm_init : main window display\n");

	  g_timeout_add(100, (GtkFunction)update_display, NULL );
	}

	void ihm_init_time( void )
	{
	  struct timeval tv;
	  char tmp[64];
	  vp_os_memset(&ihm_time,0,sizeof(ihm_time_t));
	  gettimeofday( &tv, NULL);
	  sprintf( tmp,"%d.%06d", (int)tv.tv_sec, (int)tv.tv_usec);
	  ihm_time.time_init = (double)atof( tmp );
	}

	void ihm_update_time( void )
	{
	  struct timeval tv;
	  char tmp[64];

	  gettimeofday( &tv, NULL);
	  sprintf( tmp,"%d.%06d", (int)tv.tv_sec, (int)tv.tv_usec);
	  ihm_time.time = (double)atof( tmp ) - ihm_time.time_init;
	  ihm_time.sec = (int)ihm_time.time;
	  ihm_time.min = (int)(ihm_time.sec/60);
	}


DEFINE_THREAD_ROUTINE(ihm , data)
{
	/* IHM initialisation */
	printf("\n   IHM initialisation\n\n");
	ihm_init_time( );
	pcfg = (mobile_config_t *)data;
	ihm_init();
	setlocale(LC_NUMERIC, "en_GB.UTF-8");
	//usleep(1000000);
	pcfg->ihm_curve_alloc_OK = 1;

	/* IHM main loop */
	printf("\n   IHM main loop\n\n");
	gtk_main();

	return 0;
}
