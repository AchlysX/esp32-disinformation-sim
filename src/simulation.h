#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#define GRID_SIZE 25

struct Agent {
  float opinion;
  float trust;
  float pol;
  float auth_sens;
  float disc;
  float ino;
  int   base_camp; // 0 = Camp A, 1 = Camp B
  int   state;     // 0 = A Sain, 1 = B Sain, 2 = A Infecté, 3 = B Infecté
};

// Public simulation state (read via mutex)
extern Agent          grid[GRID_SIZE][GRID_SIZE];
extern float          current_stress;
extern float          MESSAGE_OPINION;
extern int            current_mode;
extern SemaphoreHandle_t grid_mutex;

void simulation_init();
void initGrid(int mode);
void computeNextGeneration();
