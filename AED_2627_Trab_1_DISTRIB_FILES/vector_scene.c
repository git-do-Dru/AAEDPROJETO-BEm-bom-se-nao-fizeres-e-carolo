/**
 * @file vector_scene.c
 * @brief Implementation skeleton of the VectorScene ADT (2D Vector Scene)
 *
 * Algoritmos e Estruturas de Dados 2026/2027
 * Departamento de Electrónica, Telecomunicações e Informática - UA
 */

/**
 * STUDENT NAME: Alexandru Zavgorodnii
 * STUDENT NUMBER:133287
 * STUDENT NAME: Marco Oliveira
 * STUDENT NUMBER:133031
 */

#include "vector_scene.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "primitive_internal.h"

// ============================================================================
// CONCRETE STRUCT LAYOUT (OPAQUE ADT)
// ============================================================================

/**
 * @brief Concrete structure definition for VectorScene.
 * Hidden from vector_scene.h to enforce encapsulation.
 */
struct _VectorScene {
  int width;              /**< Canvas width in pixels */
  int height;             /**< Canvas height in pixels */
  Color bg_color;         /**< Default background color */
  Primitive** primitives; /**< Dynamic array of pointers to Primitive */
  size_t count;           /**< Current number of primitives in scene */
  size_t capacity;        /**< Allocated capacity for primitives array */
  int next_auto_id;       /**< Auto-increment ID generator */
};

#define INITIAL_SCENE_CAPACITY 8

// ============================================================================
// HELPER FUNCTIONS (PROVIDED TO STUDENTS)
// ============================================================================

/**
 * @brief Helper to write an RGB pixel buffer to binary PPM file (P6 format).
 */
static bool save_ppm_image(const char* filename, int width, int height,
                           const uint8_t* pixel_buffer) {
  if (!filename || !pixel_buffer || width <= 0 || height <= 0) {
    return false;
  }

  FILE* file = fopen(filename, "wb");
  if (!file) {
    return false;
  }

  fprintf(file, "P6\n%d %d\n255\n", width, height);

  size_t total_bytes = (size_t)width * height * 3;
  size_t written = fwrite(pixel_buffer, 1, total_bytes, file);
  fclose(file);

  return written == total_bytes;
}

// ============================================================================
// ACCESSORS / GETTERS (PROVIDED TO STUDENTS)
// ============================================================================

int vector_scene_get_width(const VectorScene* scene) {
  if (!scene) {
    return 0;
  }
  return scene->width;
}

int vector_scene_get_height(const VectorScene* scene) {
  if (!scene) {
    return 0;
  }
  return scene->height;
}

Color vector_scene_get_background_color(const VectorScene* scene) {
  if (!scene) {
    Color black = {0, 0, 0};
    return black;
  }
  return scene->bg_color;
}

size_t vector_scene_get_count(const VectorScene* scene) {
  if (!scene) {
    return 0;
  }
  return scene->count;
}

Primitive* vector_scene_get_primitive_by_index(const VectorScene* scene,
                                               size_t index) {
  if (!scene || index >= scene->count) {
    return NULL;
  }
  return scene->primitives[index];
}

Primitive* vector_scene_get_primitive_by_id(const VectorScene* scene, int id) {
  if (!scene || id < 0) {
    return NULL;
  }

  for (size_t i = 0; i < scene->count; i++) {
    if (scene->primitives[i] && scene->primitives[i]->id == id) {
      return scene->primitives[i];
    }
  }
  return NULL;
}

// ============================================================================
// SCENE LIFECYCLE (STUDENTS MUST COMPLETE)
// ============================================================================

VectorScene* vector_scene_create(int width, int height, Color bg_color) {
  if (width<=0 || height<=0) {
    return NULL;
  }
  
  VectorScene* scene=malloc(sizeof(VectorScene));
  if (scene == NULL){
    return NULL;
  }

  scene->width=width;
  scene->height=height;
  scene->bg_color=bg_color;
  scene->capacity=INITIAL_SCENE_CAPACITY;
  scene->count=0;
  scene->next_auto_id=0;

  scene->primitives=malloc(scene->capacity * sizeof(Primitive*));
  if (scene->primitives==NULL){
    free(scene);
    return NULL;
  }

  return scene;
}

void vector_scene_destroy(VectorScene* scene) {
  if (!scene){
    return;
  }

  for (size_t i = 0; i < scene->count; i++){
    if (scene->primitives[i]!=NULL){
      primitive_destroy(scene->primitives[i]);
    }
  }

  free(scene->primitives);
  free(scene);
}

// ============================================================================
// COLLECTION MANAGEMENT (STUDENTS MUST COMPLETE)
// ============================================================================

int vector_scene_add_primitive(VectorScene* scene, Primitive* p) {
  if (!scene || !p) {
    return -1;
  }

  //Duplica se cheio
  if (scene->count>=scene->capacity) {
    size_t new_capacity=scene->capacity*2;
    Primitive** new_primitives=realloc(scene->primitives, new_capacity * sizeof(Primitive*));
    if (new_primitives==NULL) {
      return -1;
    }
    scene->primitives=new_primitives;
    scene->capacity=new_capacity;
  }

  ///Se negativo
  if (p->id<0) {
    p->id=scene->next_auto_id++;
  } else if (p->id>=scene->next_auto_id) {
    scene->next_auto_id=p->id+1;
  }

  scene->primitives[scene->count++]=p;
  primitive_update_aabb(p);

  return p->id;
}

bool vector_scene_remove_primitive(VectorScene* scene, int id) {
  if (!scene || id < 0) {
    return false;
  }

  size_t index = (size_t)-1;
  for (size_t i=0; i<scene->count; i++) {
    if (scene->primitives[i] && scene->primitives[i]->id==id) {
      index=i;
      break;
    }
  }

  if (index == (size_t)-1) {
    return false; //ID not found
  }

  // Libertar memória da primitiva
  primitive_destroy(scene->primitives[index]);

  //Shift left
  for (size_t i=index; i<scene->count-1; i++) {
    scene->primitives[i]=scene->primitives[i+1];
  }

  scene -> count--;
  
  scene->primitives[scene -> count] = NULL;

  return true;
}

// ============================================================================
// MANIPULATION AND TRANSFORMATIONS (STUDENTS MUST COMPLETE)
// ============================================================================

bool vector_scene_translate_primitive(VectorScene* scene, int id, double dx,
                                      double dy) {
  if (!scene) {
    return false;
  }

  Primitive* p = vector_scene_get_primitive_by_id(scene, id);
  if (p == NULL) {

    return false;

  }

  primitive_translate(p, dx, dy);
  return true;
}

bool vector_scene_set_z_index(VectorScene* scene, int id, int new_z_index) {
  if (!scene) {
    return false;
  }

  Primitive* p = vector_scene_get_primitive_by_id(scene, id);
  if (p == NULL) {

    return false;
  
  }

  p->z_index = new_z_index;
  return true;
}

// ============================================================================
// SPATIAL SELECTION AND SORTING (STUDENTS MUST COMPLETE & ANALYZE)
// ============================================================================

/**
 * @brief Sorts an array of primitives by z_index using O(N^2) Insertion Sort.
 *
 * Iterative in-place sorting algorithm. Iterates through the primitive array
 * and inserts each element into its correct sorted position based on z_index.
 * Increments comparison and swap/move metrics in the SortStats structure.
 *
 * @param arr Array of Primitive pointers to sort.
 * @param count Number of elements in the array.
 * @param stats Pointer to SortStats structure to record performance metrics.
 */

static void insertion_sort_by_z(Primitive** arr, size_t count, SortStats* stats) {
  if(count<2) {
    return;
  }

  for(size_t i=1;i<count;i++) {
    Primitive* key = arr[i];
    int key_z = primitive_get_z_index(key);
    long j = (long)i - 1;

    while(j>=0) {
      if(stats!=NULL) {
        stats->comparisons++;
      }

      if(primitive_get_z_index(arr[j])>key_z) {
        arr[j + 1] = arr[j];
        if(stats!=NULL) {
          stats->swaps++;
        }
        j--;
      } else {break;}

    }
    arr[j + 1] = key;
    if(stats!=NULL) {
      stats->swaps++;
    }
  }
}

/**
 * @brief Merges two sorted subarrays of primitive pointers by z_index.
 *
 * Helper function for MergeSort that combines [left..mid] and [mid+1..right]
 * using a temporary pointer array. Increments comparisons and swaps/moves
 * counters in the SortStats structure.
 *
 * @param arr Array of Primitive pointers to merge.
 * @param temp_arr Auxiliary temporary array of Primitive pointers.
 * @param left Starting index of the first subarray.
 * @param mid Middle index separating the two subarrays.
 * @param right Ending index of the second subarray.
 * @param stats Pointer to SortStats structure to track metrics (can be NULL).
 */

static void merge_by_z(Primitive** arr, Primitive** temp_arr, size_t left, size_t mid, size_t right, SortStats* stats) {
  size_t i = left;
  size_t j = mid + 1;
  size_t k = left;

  while(i<=mid && j<=right) {
    if(stats!=NULL) {
      stats->comparisons++;
    }
    int z_i = primitive_get_z_index(arr[i]);
    int z_j = primitive_get_z_index(arr[j]);

    if(z_i<=z_j) {
      temp_arr[k++] = arr[i++];
    } else {
      temp_arr[k++] = arr[j++];
    }
    if(stats!=NULL) {
      stats->swaps++;
    }
  }

  while(i<=mid) {
    temp_arr[k++] = arr[i++];
    if(stats!=NULL) {
      stats->swaps++;
    }
  }

  while(j<=right) {
    temp_arr[k++] = arr[j++];
    if(stats!=NULL) {
      stats->swaps++;
    }
  }

  for(size_t idx=left;idx<=right;idx++) {
    arr[idx] = temp_arr[idx];
    if(stats!=NULL) {
      stats->swaps++;
    }
  }
}

/**
 * @brief Sorts a subarray of primitives by z_index using O(N log N) MergeSort.
 *
 * Stable recursive Divide-and-Conquer algorithm. Splits the array in half,
 * recursively sorts both halves, and invokes merge_by_z() to merge them.
 *
 * @param arr Array of Primitive pointers to sort.
 * @param temp_arr Auxiliary temporary array allocated for merging.
 * @param left Starting index of the subarray to sort.
 * @param right Ending index of the subarray to sort.
 * @param stats Pointer to SortStats structure to record performance metrics.
 */

static void merge_sort_by_z(Primitive** arr, Primitive** temp_arr, size_t left, size_t right, SortStats* stats) {
  if(left>=right) {
    return;
  }

  size_t mid = left + (right - left) / 2;
  merge_sort_by_z(arr, temp_arr, left, mid, stats);
  merge_sort_by_z(arr, temp_arr, mid + 1, right, stats);
  merge_by_z(arr, temp_arr, left, mid, right, stats);
}

void vector_scene_sort_by_z(VectorScene* scene, bool use_fast_sort, SortStats* stats) {
  if(scene==NULL || scene->count<2) {
    if(stats!=NULL) {
      stats->comparisons = 0;
      stats->swaps = 0;
    }
    return;
  }

  if(stats!=NULL) {
    stats->comparisons = 0;
    stats->swaps = 0;
  }

  if(!use_fast_sort) {
    insertion_sort_by_z(scene->primitives, scene->count, stats);
  } else {
    Primitive** temp_arr = malloc(scene->count * sizeof(Primitive*));
    if(temp_arr!=NULL) {
      merge_sort_by_z(scene->primitives, temp_arr, 0, scene->count - 1, stats);
      free(temp_arr);
    }
  }
}

Primitive* vector_scene_pick_top_primitive(const VectorScene* scene, Point pt, bool is_opt, PickStats* stats) {
  if(!scene) { 
    return NULL;
  }

  if(stats!=NULL) {
    stats->aabb_tests=0;
    stats->exact_tests=0;
  }

  for(long i=(long)scene->count-1;i>=0;i--) {
    Primitive* p = scene->primitives[i];
    if(!p) {
      continue;
    }

    if(is_opt) {
      if(stats!=NULL) {
        stats->aabb_tests++;
      }
      BoundingBox aabb = primitive_get_aabb(p);
      if(pt.x<aabb.min.x || pt.x>aabb.max.x || pt.y<aabb.min.y || pt.y>aabb.max.y) {
        continue;
      }
    }

    if(stats!=NULL) {
      stats->exact_tests++;
    }
    if(primitive_contains_point(p, pt)) {
      return p;
    }
  }

  return NULL;
}

// ============================================================================
// RENDERING / RASTERIZATION (STUDENTS MUST COMPLETE)
// ============================================================================

bool vector_scene_render(const VectorScene* scene, const char* output_filename) {
  
  if(!scene || !output_filename) {
    return false;
  }

  size_t buffer_size = (size_t)scene->width * (size_t)scene->height * 3;
  uint8_t* pixel_buffer = malloc(buffer_size);
  if(!pixel_buffer) {
    return false;
  }

  // Preencher com a cor de fundo
  Color bg = scene->bg_color;
  for(size_t i=0;i<(size_t)scene->width * (size_t)scene->height;i++) {
    pixel_buffer[i*3] = bg.r;
    pixel_buffer[i*3+1]=bg.g;
    pixel_buffer[i*3+2]=bg.b;
  }

  // Ordenar cópia/cena por z_index (Painter's Algorithm)
  vector_scene_sort_by_z((VectorScene*)scene, true, NULL);

  for(size_t idx=0;idx<scene->count;idx++) {
    Primitive* p = scene->primitives[idx];
    if(!p) {
      continue;
    }
    primitive_update_aabb(p);
    BoundingBox aabb=primitive_get_aabb(p);
    int x_min=(int)floor(aabb.min.x);
    int x_max=(int)ceil(aabb.max.x);
    int y_min=(int)floor(aabb.min.y);
    int y_max=(int)ceil(aabb.max.y);

    if(x_min<0) x_min=0;
    if(x_max>=scene->width) x_max=scene->width-1;
    if(y_min<0) y_min=0;
    if(y_max>=scene->height) y_max=scene->height-1;

    Color c = primitive_get_color(p);
    double alpha = primitive_get_alpha(p);

    for(int y=y_min;y<=y_max;y++) {
      for(int x=x_min;x<=x_max;x++) {
        Point pixel_center={(double)x+0.5, (double)y+0.5};
        if(primitive_contains_point(p, pixel_center)) {
          size_t p_idx=((size_t)y*(size_t)scene->width+(size_t)x)*3;
          
          uint8_t bg_r=pixel_buffer[p_idx];
          uint8_t bg_g=pixel_buffer[p_idx+1];
          uint8_t bg_b=pixel_buffer[p_idx+2];
          double final_r=alpha*c.r+(1.0-alpha)*bg_r;
          double final_g=alpha*c.g+(1.0-alpha)*bg_g;
          double final_b=alpha*c.b+(1.0-alpha)*bg_b;
          pixel_buffer[p_idx]=(uint8_t)(final_r+0.5);
          pixel_buffer[p_idx+1]=(uint8_t)(final_g+0.5);
          pixel_buffer[p_idx+2]=(uint8_t)(final_b+0.5);
        }
      }
    }
  }

  bool success = save_ppm_image(output_filename, scene->width, scene->height, pixel_buffer);
  free(pixel_buffer);
  return success;
}

// ============================================================================
// FILE INPUT / OUTPUT (PROVIDED TO STUDENTS)
// ============================================================================

bool vector_scene_save_text(const VectorScene* scene, const char* filename) {
  if (!scene || !filename) {
    return false;
  }

  FILE* file = fopen(filename, "w");
  if (!file) {
    return false;
  }

  Color bg = scene->bg_color;
  fprintf(file, "SCENE %d %d %d %d %d\n", scene->width, scene->height, bg.r,
          bg.g, bg.b);

  for (size_t i = 0; i < scene->count; i++) {
    Primitive* p = scene->primitives[i];
    if (!p) {
      continue;
    }

    Color c = p->color;

    switch (p->type) {
      case PRIM_CIRCLE:
        fprintf(file, "CIRC %d %d %d %d %d %.3f %.2f %.2f %.2f\n", p->id,
                p->z_index, c.r, c.g, c.b, p->alpha, p->geom.circle.center.x,
                p->geom.circle.center.y, p->geom.circle.radius);
        break;

      case PRIM_RECTANGLE:
        fprintf(file, "RECT %d %d %d %d %d %.3f %.2f %.2f %.2f %.2f\n", p->id,
                p->z_index, c.r, c.g, c.b, p->alpha,
                p->geom.rectangle.top_left.x, p->geom.rectangle.top_left.y,
                p->geom.rectangle.width, p->geom.rectangle.height);
        break;

      case PRIM_SEGMENT:
        fprintf(file, "SEG %d %d %d %d %d %.3f %.2f %.2f %.2f %.2f\n", p->id,
                p->z_index, c.r, c.g, c.b, p->alpha, p->geom.segment.p1.x,
                p->geom.segment.p1.y, p->geom.segment.p2.x,
                p->geom.segment.p2.y);
        break;

      case PRIM_POLYGON:
        fprintf(file, "POLY %d %d %d %d %d %.3f %zu", p->id, p->z_index, c.r,
                c.g, c.b, p->alpha, p->geom.polygon.num_vertices);

        for (size_t v = 0; v < p->geom.polygon.num_vertices; v++) {
          fprintf(file, " %.2f %.2f", p->geom.polygon.vertices[v].x,
                  p->geom.polygon.vertices[v].y);
        }
        fprintf(file, "\n");
        break;
    }
  }

  fclose(file);
  return true;
}

VectorScene* vector_scene_load_text(const char* filename) {
  if (!filename) {
    return NULL;
  }

  FILE* file = fopen(filename, "r");
  if (!file) {
    return NULL;
  }

  char line[1024];
  VectorScene* scene = NULL;

  while (fgets(line, sizeof(line), file)) {
    if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
      continue;
    }

    char tag[16];
    if (sscanf(line, "%15s", tag) != 1) {
      continue;
    }

    if (strcmp(tag, "SCENE") == 0) {
      int w, h, r, g, b;
      if (sscanf(line, "SCENE %d %d %d %d %d", &w, &h, &r, &g, &b) == 5) {
        Color bg = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
        scene = vector_scene_create(w, h, bg);
      }
    } else if (scene != NULL) {
      int id, z_index, r, g, b;
      double alpha;

      if (strcmp(tag, "CIRC") == 0) {
        double cx, cy, radius;
        if (sscanf(line, "CIRC %d %d %d %d %d %lf %lf %lf %lf", &id, &z_index,
                   &r, &g, &b, &alpha, &cx, &cy, &radius) == 9) {
          Point center = {cx, cy};
          Color color = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
          Primitive* p = primitive_create_circle(id, center, radius, color,
                                                 alpha, z_index);
          if (p) {
            vector_scene_add_primitive(scene, p);
          }
        }
      } else if (strcmp(tag, "RECT") == 0) {
        double x, y, w, h;
        if (sscanf(line, "RECT %d %d %d %d %d %lf %lf %lf %lf %lf", &id,
                   &z_index, &r, &g, &b, &alpha, &x, &y, &w, &h) == 10) {
          Point top_left = {x, y};
          Color color = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
          Primitive* p = primitive_create_rectangle(id, top_left, w, h, color,
                                                    alpha, z_index);
          if (p) {
            vector_scene_add_primitive(scene, p);
          }
        }
      } else if (strcmp(tag, "SEG") == 0) {
        double x1, y1, x2, y2;
        if (sscanf(line, "SEG %d %d %d %d %d %lf %lf %lf %lf %lf", &id,
                   &z_index, &r, &g, &b, &alpha, &x1, &y1, &x2, &y2) == 10) {
          Point p1 = {x1, y1};
          Point p2 = {x2, y2};
          Color color = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
          Primitive* p =
              primitive_create_segment(id, p1, p2, color, alpha, z_index);
          if (p) {
            vector_scene_add_primitive(scene, p);
          }
        }
      } else if (strcmp(tag, "POLY") == 0) {
        size_t num_verts;
        int offset = 0;
        int read_header = sscanf(line, "POLY %d %d %d %d %d %lf %zu%n", &id,
                                 &z_index, &r, &g, &b, &alpha, &num_verts,
                                 &offset);
        if (read_header == 7) {
          Color color = {(uint8_t)r, (uint8_t)g, (uint8_t)b};
          Primitive* p = primitive_create_polygon(id, color, alpha, z_index);
          if (p) {
            const char* ptr = line + offset;
            for (size_t v = 0; v < num_verts; v++) {
              double vx, vy;
              int step = 0;
              if (sscanf(ptr, "%lf %lf%n", &vx, &vy, &step) == 2) {
                Point pt = {vx, vy};
                polygon_add_vertex(p, pt);
                ptr += step;
              } else {
                break;
              }
            }
            vector_scene_add_primitive(scene, p);
          }
        }
      }
    }
  }

  fclose(file);
  return scene;
}