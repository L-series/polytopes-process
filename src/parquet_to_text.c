#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arrow-glib/arrow-glib.h>
#include <parquet-glib/parquet-glib.h>

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input.parquet> <output.txt>\n", argv[0]);
        return 1;
    }
    
    const char *input_file = argv[1];
    const char *output_file = argv[2];
    
    printf("Converting %s to %s\n", input_file, output_file);
    
    // Start timing
    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    // Open parquet file
    GError *error = NULL;
    GParquetArrowFileReader *reader = gparquet_arrow_file_reader_new_path(input_file, &error);
    if (error != NULL) {
        fprintf(stderr, "Failed to open parquet file: %s\n", error->message);
        g_error_free(error);
        return 1;
    }
    
    // Read the table
    GArrowTable *table = gparquet_arrow_file_reader_read_table(reader, &error);
    if (error != NULL) {
        fprintf(stderr, "Failed to read table: %s\n", error->message);
        g_error_free(error);
        g_object_unref(reader);
        return 1;
    }
    
    // Get table info
    gint64 n_rows = garrow_table_get_n_rows(table);
    gint64 n_columns = garrow_table_get_n_columns(table);
    
    printf("Table has %ld rows and %ld columns\n", n_rows, n_columns);
    
    // Open output file
    FILE *output = fopen(output_file, "w");
    if (!output) {
        fprintf(stderr, "Failed to open output file: %s\n", output_file);
        g_object_unref(table);
        g_object_unref(reader);
        return 1;
    }
    
    // Get all columns
    GArrowChunkedArray **column_arrays = malloc(n_columns * sizeof(GArrowChunkedArray*));
    GArrowArray **arrays = malloc(n_columns * sizeof(GArrowArray*));
    
    if (!column_arrays || !arrays) {
        fprintf(stderr, "Failed to allocate memory for column arrays\n");
        fclose(output);
        g_object_unref(table);
        g_object_unref(reader);
        return 1;
    }
    
    for (gint col = 0; col < n_columns; col++) {
        column_arrays[col] = garrow_table_get_column_data(table, col);
        if (!column_arrays[col]) {
            fprintf(stderr, "Failed to get column %ld\n", col);
            fclose(output);
            free(column_arrays);
            free(arrays);
            g_object_unref(table);
            g_object_unref(reader);
            return 1;
        }
        
        arrays[col] = garrow_chunked_array_get_chunk(column_arrays[col], 0);
        if (!arrays[col]) {
            fprintf(stderr, "Failed to get chunk for column %ld\n", col);
            fclose(output);
            free(column_arrays);
            free(arrays);
            g_object_unref(table);
            g_object_unref(reader);
            return 1;
        }
    }
    
    printf("Successfully got column arrays, starting conversion...\n");
    
    // Pre-cast all arrays to avoid repeated casting
    GArrowInt32Array **int32_arrays = malloc(n_columns * sizeof(GArrowInt32Array*));
    if (!int32_arrays) {
        fprintf(stderr, "Failed to allocate memory for int32 arrays\n");
        fclose(output);
        free(column_arrays);
        free(arrays);
        g_object_unref(table);
        g_object_unref(reader);
        return 1;
    }
    
    for (gint col = 0; col < n_columns; col++) {
        int32_arrays[col] = GARROW_INT32_ARRAY(arrays[col]);
    }
    
    // Use a large buffer for batch writing
    const size_t BUFFER_SIZE = 1024 * 1024; // 1MB buffer
    char *write_buffer = malloc(BUFFER_SIZE);
    if (!write_buffer) {
        fprintf(stderr, "Failed to allocate write buffer\n");
        free(int32_arrays);
        fclose(output);
        free(column_arrays);
        free(arrays);
        g_object_unref(table);
        g_object_unref(reader);
        return 1;
    }
    
    // Convert data with optimized approach
    gint64 process_rows = (n_rows > 1000000000) ? 100000 : n_rows;
    gint64 rows_written = 0;
    size_t buffer_pos = 0;
    
    for (gint64 row = 0; row < process_rows; row++) {
        // Build row string in buffer
        char row_buffer[1024]; // Assume max row length
        int row_len = 0;
        
        // Write all columns to row buffer
        for (gint col = 0; col < n_columns; col++) {
            gint32 value = 0;
            if (!garrow_array_is_null(arrays[col], row) && int32_arrays[col]) {
                value = garrow_int32_array_get_value(int32_arrays[col], row);
            }
            
            if (col > 0) {
                row_buffer[row_len++] = ' ';
            }
            row_len += sprintf(&row_buffer[row_len], "%d", value);
        }
        row_buffer[row_len++] = '\n';
        
        // Add to write buffer
        if (buffer_pos + row_len >= BUFFER_SIZE) {
            // Flush buffer
            fwrite(write_buffer, 1, buffer_pos, output);
            buffer_pos = 0;
        }
        
        memcpy(&write_buffer[buffer_pos], row_buffer, row_len);
        buffer_pos += row_len;
        rows_written++;
    }
    
    // Flush remaining buffer
    if (buffer_pos > 0) {
        fwrite(write_buffer, 1, buffer_pos, output);
    }
    
    free(write_buffer);
    free(int32_arrays);
    
    // End timing
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double elapsed = (end_time.tv_sec - start_time.tv_sec) + 
                    (end_time.tv_nsec - start_time.tv_nsec) / 1e9;
    
    printf("Conversion complete!\n");
    printf("Wrote %ld rows to %s\n", rows_written, output_file);
    printf("Time taken: %.3f seconds (%.1f rows/sec)\n", elapsed, rows_written / elapsed);
    
    // Cleanup
    fclose(output);
    
    for (gint col = 0; col < n_columns; col++) {
        g_object_unref(arrays[col]);
        g_object_unref(column_arrays[col]);
    }
    
    free(column_arrays);
    free(arrays);
    
    g_object_unref(table);
    g_object_unref(reader);
    
    return 0;
}