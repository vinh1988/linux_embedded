#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define WIDTH 800
#define HEIGHT 600
#define NUM_POINTS 100

// Function to generate a PPM image file with temperature visualization
void generate_temperature_visualization(const char* filename) {
    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to open file");
        return;
    }
    
    // PPM header
    fprintf(fp, "P6\n%d %d\n255\n", WIDTH, HEIGHT);
    
    // Allocate buffer for image data
    unsigned char* image = (unsigned char*)malloc(WIDTH * HEIGHT * 3);
    if (!image) {
        perror("Memory allocation failed");
        fclose(fp);
        return;
    }
    
    // Fill background with white
    for (int i = 0; i < WIDTH * HEIGHT * 3; i++) {
        image[i] = 255;
    }
    
    // Draw axes
    int margin = 50;
    int graph_width = WIDTH - 2 * margin;
    int graph_height = HEIGHT - 2 * margin;
    
    // X-axis
    for (int x = margin; x < WIDTH - margin; x++) {
        int y = HEIGHT - margin;
        int idx = (y * WIDTH + x) * 3;
        image[idx] = 0;
        image[idx + 1] = 0;
        image[idx + 2] = 0;
    }
    
    // Y-axis
    for (int y = margin; y < HEIGHT - margin; y++) {
        int x = margin;
        int idx = (y * WIDTH + x) * 3;
        image[idx] = 0;
        image[idx + 1] = 0;
        image[idx + 2] = 0;
    }
    
    // Generate some simulated temperature data
    srand(time(NULL));
    float temperatures[NUM_POINTS];
    float running_avg[NUM_POINTS];
    float sum = 0;
    int window_size = 5;
    
    // Generate random temperatures between 5°C and 35°C
    for (int i = 0; i < NUM_POINTS; i++) {
        temperatures[i] = 5.0f + (float)(rand() % 300) / 10.0f;
        
        // Calculate running average
        sum += temperatures[i];
        if (i >= window_size) {
            sum -= temperatures[i - window_size];
            running_avg[i] = sum / window_size;
        } else {
            running_avg[i] = sum / (i + 1);
        }
    }
    
    // Find min and max temperatures for scaling
    float min_temp = 100.0f, max_temp = -100.0f;
    for (int i = 0; i < NUM_POINTS; i++) {
        if (temperatures[i] < min_temp) min_temp = temperatures[i];
        if (temperatures[i] > max_temp) max_temp = temperatures[i];
        if (running_avg[i] < min_temp) min_temp = running_avg[i];
        if (running_avg[i] > max_temp) max_temp = running_avg[i];
    }
    
    // Add some padding to min/max
    min_temp -= 2.0f;
    max_temp += 2.0f;
    
    // Draw temperature thresholds
    float hot_threshold = 30.0f;
    float cold_threshold = 10.0f;
    
    int hot_y = HEIGHT - margin - (int)((hot_threshold - min_temp) / (max_temp - min_temp) * graph_height);
    int cold_y = HEIGHT - margin - (int)((cold_threshold - min_temp) / (max_temp - min_temp) * graph_height);
    
    // Hot threshold line (red)
    for (int x = margin; x < WIDTH - margin; x++) {
        int idx = (hot_y * WIDTH + x) * 3;
        image[idx] = 255;  // Red
        image[idx + 1] = 0;
        image[idx + 2] = 0;
    }
    
    // Cold threshold line (blue)
    for (int x = margin; x < WIDTH - margin; x++) {
        int idx = (cold_y * WIDTH + x) * 3;
        image[idx] = 0;
        image[idx + 1] = 0;
        image[idx + 2] = 255;  // Blue
    }
    
    // Draw temperature data points and lines
    for (int i = 0; i < NUM_POINTS - 1; i++) {
        int x1 = margin + (int)((float)i / NUM_POINTS * graph_width);
        int y1 = HEIGHT - margin - (int)((temperatures[i] - min_temp) / (max_temp - min_temp) * graph_height);
        int x2 = margin + (int)((float)(i + 1) / NUM_POINTS * graph_width);
        int y2 = HEIGHT - margin - (int)((temperatures[i + 1] - min_temp) / (max_temp - min_temp) * graph_height);
        
        // Draw line between points
        float dx = x2 - x1;
        float dy = y2 - y1;
        float steps = fmaxf(fabsf(dx), fabsf(dy));
        float x_inc = dx / steps;
        float y_inc = dy / steps;
        
        float x = x1;
        float y = y1;
        
        for (int j = 0; j <= steps; j++) {
            int px = (int)x;
            int py = (int)y;
            if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {
                int idx = (py * WIDTH + px) * 3;
                image[idx] = 0;
                image[idx + 1] = 0;
                image[idx + 2] = 0;
            }
            x += x_inc;
            y += y_inc;
        }
    }
    
    // Draw running average in green
    for (int i = 0; i < NUM_POINTS - 1; i++) {
        int x1 = margin + (int)((float)i / NUM_POINTS * graph_width);
        int y1 = HEIGHT - margin - (int)((running_avg[i] - min_temp) / (max_temp - min_temp) * graph_height);
        int x2 = margin + (int)((float)(i + 1) / NUM_POINTS * graph_width);
        int y2 = HEIGHT - margin - (int)((running_avg[i + 1] - min_temp) / (max_temp - min_temp) * graph_height);
        
        // Draw line between points
        float dx = x2 - x1;
        float dy = y2 - y1;
        float steps = fmaxf(fabsf(dx), fabsf(dy));
        float x_inc = dx / steps;
        float y_inc = dy / steps;
        
        float x = x1;
        float y = y1;
        
        for (int j = 0; j <= steps; j++) {
            int px = (int)x;
            int py = (int)y;
            if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {
                int idx = (py * WIDTH + px) * 3;
                image[idx] = 0;
                image[idx + 1] = 128;  // Green
                image[idx + 2] = 0;
            }
            x += x_inc;
            y += y_inc;
        }
    }
    
    // Add labels
    // (In a real application, you would use a proper font rendering library)
    // Here we'll just add some markers
    
    // Write the image data
    fwrite(image, 1, WIDTH * HEIGHT * 3, fp);
    
    // Clean up
    free(image);
    fclose(fp);
    
    printf("Temperature visualization saved to %s\n", filename);
}

int main() {
    generate_temperature_visualization("temperature_visualization.ppm");
    
    // Convert PPM to PNG using system command (requires ImageMagick)
    system("convert temperature_visualization.ppm temperature_measurement.png");
    system("rm temperature_visualization.ppm");
    
    return 0;
}
