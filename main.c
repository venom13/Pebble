#include <pebble.h>

#define TEMPERATURE 0
#define CONDITIONS 1
#define LOCATION 2

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *date_layer;
static TextLayer *s_weather_layer;
static TextLayer *battery_layer;
static TextLayer *steps_layer;
static GFont s_time_font;
static GFont date_font;
static GFont s_weather_font;
static BitmapLayer *s_background_layer;

void update_steps(int steps) {
  if(steps > 0) {
    static char s_buffer[32];
    snprintf(s_buffer, sizeof(s_buffer), "Adım Sayısı: %d", steps);
    text_layer_set_text(steps_layer, s_buffer);
  } else {
    text_layer_set_text(steps_layer, "Adım Sayısı: 0");
  }
}

static void health_handler(HealthEventType event, void *context) {
  // Update step count
  HealthMetric metric = HealthMetricStepCount;
  HealthServiceAccessibilityMask result = 
    health_service_metric_accessible(metric, time_start_of_today(), time(NULL));
  int steps = 0;
  if(result == HealthServiceAccessibilityMaskAvailable) {
    steps = (int)health_service_sum_today(metric);
  }
  update_steps(steps);
}

static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100%";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "<+>");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  }
  text_layer_set_text(battery_layer, battery_text);
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char location_buffer[32];
  static char weather_layer_buffer[32];

  // Read tuples for data
  Tuple *temp_tuple = dict_find(iterator, TEMPERATURE);
  Tuple *conditions_tuple = dict_find(iterator, CONDITIONS);
  Tuple *location_tuple = dict_find(iterator, LOCATION);

  // If all data is available, use it
  if(temp_tuple && conditions_tuple) {
    snprintf(temperature_buffer, sizeof(temperature_buffer), "%d°C", (int)temp_tuple->value->int32);
    snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);
    snprintf(location_buffer, sizeof(location_buffer), "%s", location_tuple->value->cstring);
    
    if (strcmp(conditions_buffer,"Clouds")==0) {
      strcpy(conditions_buffer, "Bulutlu");
    }
    else if (strcmp(conditions_buffer,"Mist")==0) {
      strcpy(conditions_buffer, "Sisli");
    }
    else if (strcmp(conditions_buffer,"Rain")==0) {
      strcpy(conditions_buffer, "Yağmurlu");
    }
    
    // Assemble full string and display
    snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s\n%s", temperature_buffer, conditions_buffer, location_buffer);
    text_layer_set_text(s_weather_layer, weather_layer_buffer);
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void update_date() {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

//GÜN---------------- ----------------------------------------------------------------
  static char day[3];
  strftime(day, sizeof(day), "%d", tick_time);
  
//AY----------------------------------------------------------------------------------
  static char month[10];
  
  if (tick_time->tm_mon == 0) {
    strcpy(month,"Ocak");
  }
  else if (tick_time->tm_mon == 1) {
    strcpy(month,"Şubat");
  }
  else if (tick_time->tm_mon == 2) {
    strcpy(month,"Mart");
  }
  else if (tick_time->tm_mon == 3) {
    strcpy(month,"Nisan");
  }
  else if (tick_time->tm_mon == 4) {
    strcpy(month,"Mayıs");
  }
  else if (tick_time->tm_mon == 5) {
    strcpy(month,"Haziran");
  }
  else if (tick_time->tm_mon == 6){
    strcpy(month,"Temmuz");
  }
  else if (tick_time->tm_mon == 7){
    strcpy(month,"Ağustos");
  }
  else if (tick_time->tm_mon == 8){
    strcpy(month,"Eylül");
  }
  else if (tick_time->tm_mon == 9){
    strcpy(month,"Ekim");
  }
  else if (tick_time->tm_mon == 10){
    strcpy(month,"Kasım");
  }
  else if (tick_time->tm_mon == 11){
    strcpy(month,"Aralık");
  }
  else {
    strftime(month, sizeof(month), "%b", tick_time);
  };

//YIL---------------------------------------------------------------------------------
  static char year[5];
  strftime(year, sizeof(year), "%Y", tick_time);
  
//HAFTANIN GÜNÜ-----------------------------------------------------------------------
  static char day_of_the_week[10];
  
  if (tick_time->tm_wday == 0) {
    strcpy(day_of_the_week,"Pazar");
  }
  else if (tick_time->tm_wday == 1) {
    strcpy(day_of_the_week,"Pazartesi");
  }
  else if (tick_time->tm_wday == 2) {
    strcpy(day_of_the_week,"Salı");
  }
  else if (tick_time->tm_wday == 3) {
    strcpy(day_of_the_week,"Çarşamba");
  }
  else if (tick_time->tm_wday == 4) {
    strcpy(day_of_the_week,"Perşembe");
  }
  else if (tick_time->tm_wday == 5) {
    strcpy(day_of_the_week,"Cuma");
  }
  else if (tick_time->tm_wday == 6){
    strcpy(day_of_the_week,"Cumartesi");
  }
  else {
    strftime(day_of_the_week, sizeof(day_of_the_week), "%A", tick_time);
  };
  
  static char tarih[30];
  strcpy(tarih, day);
  strcat(tarih, " ");
  strcat(tarih, month);
  strcat(tarih, " ");
  strcat(tarih, year);
  strcat(tarih, "\n");
  strcat(tarih, day_of_the_week);
  
  text_layer_set_text(date_layer, tarih);
}

static int power_saving_time() {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  if ((tick_time->tm_hour >= 0) & (tick_time->tm_hour <= 6))
    return 1;
  else
    return 0;
}

static void update_time() {
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[12];
  strftime(s_buffer, sizeof(s_buffer), power_saving_time() ? // clock_is_24h_style() ?
                                          "%H:%M" : "%H:%M:%S", tick_time);
  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  if(tick_time->tm_hour % 24 == 0) {
    update_date();
  }
  
  // Get weather update every 20 minutes
  if(tick_time->tm_min % 20 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create BitmapLayer to display the GBitmap
  s_background_layer = bitmap_layer_create(bounds);
  layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));  
  
  battery_layer = text_layer_create(GRect(112, 0, 30, 14));
  text_layer_set_text_color(battery_layer, GColorBlack);
  text_layer_set_background_color(battery_layer, GColorClear);
  text_layer_set_font(battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_alignment(battery_layer, GTextAlignmentRight);
  text_layer_set_text(battery_layer, "100%");
  battery_state_service_subscribe(handle_battery);
  layer_add_child(window_layer, text_layer_get_layer(battery_layer));
  handle_battery(battery_state_service_peek());
  
  date_layer = text_layer_create(GRect(0, 8, 144, 40));
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_text_color(date_layer, GColorBlack);
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(date_layer));
  date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_Phenomena_Bold_20));
  text_layer_set_font(date_layer, date_font);

  s_time_layer = text_layer_create(GRect(0, 52, 144, 53));
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_Phenomena_ExtraBold_42));
  text_layer_set_font(s_time_layer, s_time_font);
  
  s_weather_layer = text_layer_create(GRect(0, 104, 144, 44));
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_text_color(s_weather_layer, GColorBlack);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer, "Loading...");
  s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_Phenomena_ExtraBold_20));
  text_layer_set_font(s_weather_layer, s_weather_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));
  
  steps_layer = text_layer_create(GRect(0, 150, 144, 18));
  text_layer_set_background_color(steps_layer, GColorBlack);
  text_layer_set_text_color(steps_layer, GColorWhite);
  text_layer_set_text_alignment(steps_layer, GTextAlignmentCenter);
  text_layer_set_font(steps_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(steps_layer));
}

static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(date_layer);
  text_layer_destroy(steps_layer);
  
  // Unload GFont
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(date_font);
  
  // Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
  
  // Destroy weather elements
  text_layer_destroy(s_weather_layer);
  fonts_unload_custom_font(s_weather_font);
  
  battery_state_service_unsubscribe();
  text_layer_destroy(battery_layer);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  window_set_background_color(s_main_window, GColorCadetBlue);

  // Make sure the time is displayed from the start
  update_time();
  update_date();

  // Register with TickTimerService
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
  
  // Subscribe to health-related events
  health_service_events_subscribe(health_handler, NULL);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
  
  if(!health_service_events_subscribe(health_handler, NULL)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Health not available!");
  }
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
