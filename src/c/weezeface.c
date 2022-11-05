/*
  weezeface - Pebble Watch Face

  Created by John Spahr (johnspahr.org)

  Thanks to: "PebbleFaces" example, the watchface creation guide hosted by Rebble, Rivers Cuomo, and the 2022 Rebble Hackathon team!

  One more thing: The Weezer covers are not my own art, obviously. I'm using them for parody purposes.
*/
#include <pebble.h>

static Window *window;            // window object
static TextLayer *s_time_layer;   // the clock (text layer)
static BitmapLayer *bitmap_layer; // layer to display the bitmap
static Layer *s_battery_layer;    // layer to display battery level
static GBitmap *current_bmp;      // current bitmap object
static GBitmap *mono_weezer;   // mono weezer bitmap
static GBitmap *color_weezer;  // color weezer bitmap
static GBitmap *round_weezer;  // round weezer bitmap
static GBitmap *mono_diss;       // mono disconnect weezer bitmap
static GBitmap *color_diss;      // color disconnect weezer bitmap
static GBitmap *round_diss;      // round disconnect weezer bitmap
static GFont s_time_font;         // custom font
static int s_battery_level;       // battery level value

static void update_time()
{
  // get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  // display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
  update_time(); // update the time on timer tick
}

static void show_weezer()
{
  // show default weezer bitmap
  switch (PBL_PLATFORM_TYPE_CURRENT)
  {
  // detect platform...
  case PlatformTypeAplite:
  case PlatformTypeDiorite:
    // monochrome pebbles
    if (!mono_weezer)
    {
      mono_weezer = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MONO_WEEZER); // create bitmap from image resource if it doesn't exist yet
    }
    bitmap_layer_set_bitmap(bitmap_layer, mono_weezer); // set bitmap layer image
    current_bmp = mono_weezer;                          // update current bitmap object
    break;
  case PlatformTypeBasalt:
    // color (rectangular) pebbles
    if (!color_weezer)
    {
      color_weezer = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_COLOR_WEEZER); // create bitmap from image resource if it doesn't exist yet
    }
    bitmap_layer_set_bitmap(bitmap_layer, color_weezer); // set bitmap layer image
    current_bmp = color_weezer;                          // update current bitmap object
    break;
  case PlatformTypeChalk:
    // pebble time round
    if (!round_weezer)
    {
      round_weezer = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ROUND_WEEZER); // create bitmap from image resource if it doesn't exist yet
    }
    bitmap_layer_set_bitmap(bitmap_layer, round_weezer); // set bitmap layer image
    current_bmp = round_weezer;                          // update current bitmap object
    break;
  }
}

static void bluetooth_callback(bool connected)
{
  if (connected)
  {
    show_weezer(); // show nice weezer if connected
  }
  else
  {
    // otherwise, show disconnect weezer bitmap
    vibes_double_pulse(); // issue a vibrating alert on disconnect

    switch (PBL_PLATFORM_TYPE_CURRENT)
    {
    // detect platform...
    case PlatformTypeAplite:
    case PlatformTypeDiorite:
      // monochrome pebbles
      if (!mono_diss)
      {
        mono_diss = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MONO_DISS); // create bitmap from image resource if it doesn't exist yet
      }
      bitmap_layer_set_bitmap(bitmap_layer, mono_diss); // set bitmap layer image
      current_bmp = mono_diss;                          // update current bitmap object
      break;
    case PlatformTypeBasalt:
      // color (rectangular) pebbles
      if (!color_diss)
      {
        color_diss = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_COLOR_DISS); // create bitmap from image resource if it doesn't exist yet
      }
      bitmap_layer_set_bitmap(bitmap_layer, color_diss); // set bitmap layer image
      current_bmp = color_diss;                          // update current bitmap object
      break;
    case PlatformTypeChalk:
      // pebble time round
      if (!round_diss)
      {
        round_diss = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ROUND_DISS); // create bitmap from image resource if it doesn't exist yet
      }
      bitmap_layer_set_bitmap(bitmap_layer, round_diss); // set bitmap layer image
      current_bmp = round_diss;                          // update current bitmap object
      break;
    }
  }
}

static void window_load(Window *window)
{
  Layer *window_layer = window_get_root_layer(window); // get current window layer
  GRect bounds = layer_get_bounds(window_layer);       // get screen bounds

  bitmap_layer = bitmap_layer_create(bounds);                          // create bitmap layer
  layer_add_child(window_layer, bitmap_layer_get_layer(bitmap_layer)); // add bitmap layer to window

  // create GFont for clock
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_CGOTHIC_28));

  // create clock text layer...
  s_time_layer = text_layer_create(
      GRect(0, PBL_IF_ROUND_ELSE(20, 16), bounds.size.w, bounds.size.h));

  // time text layer setup...
  text_layer_set_background_color(s_time_layer, GColorClear);        // set clear background color
  text_layer_set_text_color(s_time_layer, GColorBlack);              // black text color
  text_layer_set_text(s_time_layer, "00:00");                        // set text to 00:00
  text_layer_set_font(s_time_layer, s_time_font);                    // set font to Century Gothic, 28pt
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter); // center align text

  // add clock text layer to window layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  // load default weezer bitmap
  show_weezer();
}

static void window_unload(Window *window)
{
  // get rid of bitmap layer
  bitmap_layer_destroy(bitmap_layer);

  // dispose of bitmaps if they exist...
  if (current_bmp)
  {
    gbitmap_destroy(current_bmp); // current bitmap
  }

  // destroy time text layer
  text_layer_destroy(s_time_layer);

  // destroy battery layer
  layer_destroy(s_battery_layer);

  // unload GFont
  fonts_unload_custom_font(s_time_font);
}

static void init(void)
{
  window = window_create(); // create a new window. woohoo!
  window_set_window_handlers(window, (WindowHandlers){
                                         .load = window_load,
                                         .unload = window_unload,
                                     }); // set functions for window handlers

  window_stack_push(window, true); // push window

  update_time(); // register with TickTimerService

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler); // set tick_handler function as time handler (start keeping time, essentially. kinda important on a watch.)

  // register for Bluetooth connection updates
  connection_service_subscribe((ConnectionHandlers){
      .pebble_app_connection_handler = bluetooth_callback});
}

static void deinit(void)
{
  window_destroy(window); // literally obliterate the window
}

int main(void)
{
  init();
  app_event_loop();
  deinit();
}