#include "simulation.h"

Agent           grid[GRID_SIZE][GRID_SIZE];
static Agent    next_grid[GRID_SIZE][GRID_SIZE]; // private to this file
float           current_stress  = 0.0f;
float           MESSAGE_OPINION = 1.0f;
int             current_mode    = 1;
SemaphoreHandle_t grid_mutex;

void simulation_init() {
  grid_mutex = xSemaphoreCreateMutex();
  configASSERT(grid_mutex); // crash early if allocation fails
  randomSeed(esp_random()); // hardware RNG — no more deterministic boots
}

void initGrid(int mode) {
  for (int x = 0; x < GRID_SIZE; x++) {
    for (int y = 0; y < GRID_SIZE; y++) {
      if (random(0, 100) < 60) {
        grid[x][y].opinion   = random(10, 40)  / 100.0f;
        grid[x][y].trust     = random(60, 95)  / 100.0f;
        grid[x][y].pol       = random(10, 30)  / 100.0f;
        grid[x][y].base_camp = 0;
        grid[x][y].state     = 0;
      } else {
        grid[x][y].opinion   = random(60, 90)  / 100.0f;
        grid[x][y].trust     = random(10, 40)  / 100.0f;
        grid[x][y].pol       = random(70, 95)  / 100.0f;
        grid[x][y].base_camp = 1;
        grid[x][y].state     = 1;
      }
      grid[x][y].disc = random(40, 90) / 100.0f;
      grid[x][y].ino  = random(10, 50) / 100.0f;
    }
  }

  // Injection
  if (mode == 1) {
    int rx = random(2, GRID_SIZE - 2);
    int ry = random(2, GRID_SIZE - 2);
    grid[rx][ry].state   = (grid[rx][ry].base_camp == 0) ? 2 : 3;
    grid[rx][ry].opinion = MESSAGE_OPINION;
  } else if (mode == 2) {
    for (int i = 0; i < 6; i++) {
      int rx, ry;
      do {
        rx = random(0, GRID_SIZE);
        ry = random(0, GRID_SIZE);
      } while (grid[rx][ry].state >= 2);
      grid[rx][ry].state   = (grid[rx][ry].base_camp == 0) ? 2 : 3;
      grid[rx][ry].opinion = MESSAGE_OPINION;
    }
  }
}

void computeNextGeneration() {
  // Take a local snapshot of volatile params to avoid mid-tick changes
  const float stress  = current_stress;
  const float message = MESSAGE_OPINION;

  for (int x = 0; x < GRID_SIZE; x++) {
    for (int y = 0; y < GRID_SIZE; y++) {
      Agent me        = grid[x][y];
      next_grid[x][y] = me;

      if (me.state >= 2) continue;

      int infected_neighbors = 0;
      int total_neighbors    = 0;

      for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
          if (i == 0 && j == 0) continue;
          int nx = x + i, ny = y + j;
          if (nx >= 0 && nx < GRID_SIZE && ny >= 0 && ny < GRID_SIZE) {
            total_neighbors++;
            if (grid[nx][ny].state >= 2) infected_neighbors++;
          }
        }
      }

      if (total_neighbors == 0 || infected_neighbors == 0) continue;

      const float disc_eff         = me.disc * (1.0f - 0.8f * stress);
      const float social_pressure  = (float)infected_neighbors / total_neighbors;
      const float rational_defense = (disc_eff + me.ino) * (1.0f - stress);
      const float ideological_pull = me.pol * (1.0f - fabsf(me.opinion - message)); // fabsf, not abs
      const float social_drive     = social_pressure * (1.0f + stress);
      const float logit            = ideological_pull + social_drive - rational_defense - me.trust;
      const float probability      = 1.0f / (1.0f + expf(-5.0f * logit));

      if (stress > 0.95f) {
        next_grid[x][y].state = (me.base_camp == 0) ? 2 : 3;
      } else {
        const float chance = random(0, 10000) / 10000.0f; // finer resolution than /100
        if (chance < probability) {
          next_grid[x][y].state = (me.base_camp == 0) ? 2 : 3;
        }
      }
    }
  }

  // Commit next_grid → grid under mutex
  if (xSemaphoreTake(grid_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
    memcpy(grid, next_grid, sizeof(grid));
    xSemaphoreGive(grid_mutex);
  }
}
