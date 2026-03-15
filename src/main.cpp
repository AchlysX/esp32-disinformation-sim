#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"
#include "simulation.h"
#include "web_server.h"

// ── Simulation task (FreeRTOS) ─────────────────────────────────────────────────
static void sim_task(void* pvParameters) {
  const TickType_t period = pdMS_TO_TICKS(100);
  TickType_t       last_wake = xTaskGetTickCount();

  while (true) {
    computeNextGeneration();
    vTaskDelayUntil(&last_wake, period); // precise 100ms ticks, not drift
  }
}

void setup() {
  Serial.begin(115200);

  simulation_init();         // mutex + RNG seed
  initGrid(current_mode);

  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  webserver_init();

  // Pin sim task to Core 0, HTTP server runs on Core 1 (Arduino default)
  xTaskCreatePinnedToCore(
    sim_task,
    "sim_task",
    4096,   // stack size in bytes
    NULL,
    1,      // priority
    NULL,
    0       // Core 0
  );
}

void loop() {
  server.handleClient(); // runs on Core 1
}
