/*

Importing a tile map
Richard Turnnidge, Dec 2025

Example of:
  1) using a buffer to load a RGBA2222 colour bitmap 
  2) load in chunks from file
  3) split chunks into smaller bitmaps stored in buffers

Image format is RGBA2222, therefore, 1 byte per pixel
Our source tile map image has 8 x 5 cells, therefore 128 x 80 pixels

            <- Columns ->
+---+---+---+---+---+---+---+---+
| 0 |   |   |   |   |   |   | 7 | 
+---+---+---+---+---+---+---+---+
|   |   |   |   |   |   |   |   |   ^
+---+---+---+---+---+---+---+---+   |
|   |   |   |   |   |   |   |   |   Rows
+---+---+---+---+---+---+---+---+   |
|   |   |   |   |   |   |   |   |   v
+---+---+---+---+---+---+---+---+
| 24|   |   |   |   |   |   | 31|
+---+---+---+---+---+---+---+---+
Image contains 32 cells
                            
+---+---+---+---+---+---+---+---+
| 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 |   Load file in 2048 byte chunks
+---+---+---+---+---+---+---+---+

    16
   +---+
16 |   |   Split into 256 bytes for a 16 x 16 pixel bitmap
   +---+

In this example, the newly created bitmaps will be stored in buffer IDs from 0 - 31

*/

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <agon/vdp.h>
#include <agon/timer.h>

const uint8_t  screen_mode = 8;
const uint8_t  RGBA2222_format = 1;

const uint16_t chunkSize = 2048;
const uint16_t chunkStoreID = 5000;
const uint16_t tileStartID = 0;

const uint16_t cellWidth = 16;
const uint16_t cellHeight = 16;
const uint16_t cellRows = 5;
const uint16_t cellColumns = 8;

char fname[]="basictiles_merged.RGB2";        // file we are going to open
char readChunk[2048];                         // for storing chunks of file data

// functions
void load_tiles_from_file(char fileName[], uint16_t startBitmapID, uint16_t cellWidth, uint16_t cellHeight, uint16_t rowCount, uint16_t blockCount );
void display_tiles(uint8_t rows, uint8_t columns, uint16_t startBufferID);

// ---------------------------------------------------------------------------------
// main program

int main(void) {

  // setup default parameters
  vdp_mode(screen_mode);                      // set to screen mode 8 (65 colours)
  vdp_cursor_enable(false);                   // hide the cursor
  vdp_set_text_bg_colour(BLUE);
  vdp_clear_screen();                         // clear the screen
  vdp_set_pixel_coordinates();                // use pixel coordinates

  // print a title on the screen
  vdp_cursor_tab(0, 27);                      // tab to a position
  printf("AgonDev Tile Map example");         // and draw some text
  vdp_cursor_tab(0, 29);
  printf("Press ESC to exit");

  // load data from file and distribute to bitmap buffers
  load_tiles_from_file(fname, tileStartID, cellWidth, cellHeight, cellRows, cellColumns);   

  // display the bitmaps on the screen in a nice grid
  display_tiles(5, 8, tileStartID);                            

  // just wait for user to press ESC
  while(true) {
    if(vdp_getKeyCode() == 27) break;         // continue only if ESC key pressed
  }
  // tidy up and exit
  vdp_set_text_bg_colour(0);
  vdp_clear_screen();                         // clear the screen
  vdp_cursor_enable(true);                    // show text cursor
  return 0;                                   // exit to MOS
}

// ---------------------------------------------------------------------------------
// read file a chunk at a time
// with each chunk, split width into sections and save as new buffers
// then convert to bitmaps

void load_tiles_from_file(char fileName[], uint16_t startBitmapID, uint16_t cellWidth, uint16_t cellHeight, uint16_t rowCount, uint16_t blockCount ){
 
  FILE *tileMapFile = fopen(fileName, "r");                         // open the RGBA2222 image file

  for(uint8_t chunkLoop = 0; chunkLoop < rowCount; chunkLoop ++){   // loop round reading 'chunkSize' pieces of file data 
    vdp_adv_clear_buffer(chunkStoreID);                             // clear the buffer
    fread(readChunk, 1, chunkSize, tileMapFile);                    // read 2048 byte chunk of data from file into 'readChunk[]'
    vdp_adv_write_block_data(chunkStoreID, chunkSize, readChunk);   // send chunk into VDP buffer 'chunkStoreID'

    // we now have data for a chunk stored in buffer 'chunkStoreID'
    vdp_adv_split_by_width_multiple_from(chunkStoreID, cellWidth, blockCount, startBitmapID);  // split buffer into new buffers

    // we now have 8 new buffers, but need to be converted to bitmaps before use
    for(uint8_t rowLoop = 0; rowLoop < blockCount; rowLoop ++){              // loop through all buffers and make bitmaps
      vdp_adv_select_bitmap(startBitmapID);                                  // select this one
      vdp_adv_bitmap_from_buffer(cellWidth, cellHeight, RGBA2222_format);    // make a bitmap
      startBitmapID ++;      
    }
  }
  fclose(tileMapFile);                                               // close the file
}

// ---------------------------------------------------------------------------------
// display all seperate 16x16 bitmaps on teh screen in a grid

void display_tiles(uint8_t rows, uint8_t columns, uint16_t startBufferID){

  for(uint8_t rLoop = 0; rLoop < rows; rLoop ++){             // loop through all buffers and make bitmaps
    for(uint8_t cLoop = 0; cLoop < columns; cLoop ++){        // loop through all buffers and make bitmaps
      vdp_adv_select_bitmap(startBufferID);                   // select this one
      vdp_plot_bitmap(10 + (cLoop * 20), 10 + (rLoop * 20));  // plot the bitmap, giving a little space between each
      startBufferID ++;                                       // increase bitmap counter  
    }                  
  }
}

// ---------------------------------------------------------------------------------

