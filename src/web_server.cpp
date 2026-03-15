#include "web_server.h"
#include "simulation.h"
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "secrets.h"  


WebServer server(80);

// ── Auth helper ────────────────────────────────────────────────────────────────
static bool isAuthorized() {
  if (server.hasHeader("X-Token") && server.header("X-Token") == API_TOKEN)
    return true;
  // Also accept token as query param for easy browser testing
  if (server.hasArg("token") && server.arg("token") == API_TOKEN)
    return true;
  server.send(401, "text/plain", "Unauthorized");
  return false;
}

// ── Rate limiting ──────────────────────────────────────────────────────────────
static unsigned long last_data_request = 0; 
static const unsigned long DATA_RATE_LIMIT_MS = 100; // max 10 req/s per server

// ── Handlers ──────────────────────────────────────────────────────────────────
static void handleRoot() {
  server.sendHeader("Content-Type", "text/html; charset=utf-8");
  server.sendHeader("Cache-Control", "no-cache");
  server.send(200, "text/html", "");           // LittleFS serves the file
}

static void handleData() {
  unsigned long now = millis();
  if (now - last_data_request < DATA_RATE_LIMIT_MS) {
    server.send(429, "text/plain", "Too Many Requests");
    return;
  }
  last_data_request = now;

  JsonDocument doc;
  doc["stress"] = current_stress;
  doc["target"] = MESSAGE_OPINION;
  doc["mode"]   = current_mode;

  JsonArray arr = doc["grid"].to<JsonArray>();

  // Take mutex — safe read of grid
  if (xSemaphoreTake(grid_mutex, pdMS_TO_TICKS(20)) == pdTRUE) {
    for (int y = 0; y < GRID_SIZE; y++)
      for (int x = 0; x < GRID_SIZE; x++)
        arr.add(grid[x][y].state);
    xSemaphoreGive(grid_mutex);
  }

  String response;
  serializeJson(doc, response);
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, "application/json", response);
}

static void handleReset() {
  if (!isAuthorized()) return;
  if (server.hasArg("mode")) current_mode = server.arg("mode").toInt();
  current_stress = 0.0f;
  if (xSemaphoreTake(grid_mutex, portMAX_DELAY) == pdTRUE) {
    initGrid(current_mode);
    xSemaphoreGive(grid_mutex);
  }
  server.send(200, "text/plain", "OK");
}

static void handleSetStress() {
  if (!isAuthorized()) return;
  if (server.hasArg("value")) {
    float val = server.arg("value").toFloat();
    // Clamp to [0, 1]
    current_stress = constrain(val, 0.0f, 1.0f);
  }
  server.send(200, "text/plain", "OK");
}

static void handleSetTarget() {
  if (!isAuthorized()) return;
  if (server.hasArg("val")) {
    float val = server.arg("val").toFloat();
    MESSAGE_OPINION = constrain(val, 0.0f, 1.0f);
  }
  server.send(200, "text/plain", "OK");
}

static void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

void webserver_init() {
  if (!LittleFS.begin()) {
    // Fallback: LittleFS not formatted — format once and restart
    LittleFS.format();
    ESP.restart();
  }

  // Serve static files (index.html, theorie.html, CSS, JS)
  server.serveStatic("/",        LittleFS, "/index.html");
  server.serveStatic("/theorie", LittleFS, "/theorie.html");

  // Collect headers for auth
  const char* headerKeys[] = {"X-Token"};
  server.collectHeaders(headerKeys, 1);

  server.on("/data",       HTTP_GET, handleData);
  server.on("/reset",      HTTP_GET, handleReset);
  server.on("/set_stress", HTTP_GET, handleSetStress);
  server.on("/set_target", HTTP_GET, handleSetTarget);
  server.onNotFound(handleNotFound);

  server.begin();
}
