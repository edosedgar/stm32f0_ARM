#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>


#define BLACK 0
#define WHITE 1
#define OTHER -1

#define TRUE  1
#define FALSE 0

typedef struct {
    unsigned short int type;                 /* Magic identifier            */
    unsigned int size;                       /* File size in bytes          */
    unsigned short int reserved1, reserved2;
    unsigned int offset;                     /* Offset to image data, bytes */
} HEADER;

typedef struct {
    unsigned int size;               /* Header size in bytes      */
    int width, height;                /* Width and height of image */
    unsigned short int planes;       /* Number of colour planes   */
    unsigned short int bits;         /* Bits per pixel            */
    unsigned int compression;        /* Compression type          */
    unsigned int imagesize;          /* Image size in bytes       */
    int xresolution,yresolution;     /* Pixels per meter          */
    unsigned int ncolours;           /* Number of colours         */
    unsigned int importantcolours;   /* Important colours         */
} INFOHEADER;

typedef struct {
    unsigned char r,g,b,a;
} COLOURTABLE;

typedef struct {
    char* bitmap;
    int height;
    int width;
} BITMAP_DATA;

int ReadUShort(FILE *,unsigned short *,int);
int ReadUInt(FILE *,unsigned int *,int);

/* 
 * @ Read BMP file @
 *  
 * Parse BMP file at src_fptr.
 * Write bitmap BLACK/WHITE/OTHER to dest_bitmapdata.
 * Print log to stderr.
 * Return 0 on success, otherwise -1.
 * */
int read_bmp_file(FILE *src_fptr, BITMAP_DATA *dest_bitmapdata, int *max_width, int *max_height)
{
    int i, j, row_index, height_i;
    int padding;
    unsigned char colour_index;
    HEADER header;
    INFOHEADER infoheader;
    COLOURTABLE *colourtable;
    
    if ((colourtable = (COLOURTABLE*)malloc(256*sizeof(COLOURTABLE))) == NULL) {
        printf(" ERROR: Memory allocation failed. Exiting.\n");
        exit(-1);
    }
    *max_width = 0; *max_height = 0;
    
    /* Read and check the header */
    
    ReadUShort(src_fptr, &header.type, FALSE);
    if (header.type != 'M'*256+'B') {
        fprintf(stderr, "\n WARNING: It looks like this file is not BMP. ID is: %d, should be %d. Skipping file.\n", header.type,'M'*256+'B');
        free(colourtable);
        return -1;
    }
    
    ReadUInt(src_fptr, &header.size,FALSE);
    ReadUShort(src_fptr, &header.reserved1, FALSE);
    ReadUShort(src_fptr, &header.reserved2, FALSE);
    ReadUInt(src_fptr, &header.offset, FALSE);

    /* Read and check the information header */
    
    if (fread(&infoheader,sizeof(INFOHEADER),1,src_fptr) != 1) {
        fprintf(stderr,"\n WARNING: Failed to read BMP info header. Skipping file.\n");
        free(colourtable);
        return -1;
    }
    
    if (infoheader.bits != 8) {
        fprintf(stderr, "\n WARNING: Bits per pixel is %d, expected 8. Skipping file.\n", infoheader.bits);
        free(colourtable);
        return -1;
    }
    if (infoheader.compression != 0) {
        fprintf(stderr, "\n WARNING: File is compressed. Expected uncompressed file.\n");
        free(colourtable);
        return -1;
    }
    
    for (i = 0; i < infoheader.ncolours; i++) {
        if (fread(&colourtable[i].b, sizeof(unsigned char), 1, src_fptr) != 1) {
            fprintf(stderr,"\n WARNING: Colour table read failed. Skipping file.\n");
            free(colourtable);
            return -1;
        }
        if (fread(&colourtable[i].g, sizeof(unsigned char), 1, src_fptr) != 1) {
            fprintf(stderr,"\n WARNING: Colour table read failed. Skipping file.\n");
            free(colourtable);
            return -1;
        }
        if (fread(&colourtable[i].r, sizeof(unsigned char), 1, src_fptr) != 1) {
            fprintf(stderr,"\n WARNING: Colour table read failed. Skipping file.\n");
            free(colourtable);
            return -1;
        }
        if (fread(&colourtable[i].a, sizeof(unsigned char), 1, src_fptr) != 1) {
            fprintf(stderr,"\n WARNING: Colour table read failed. Skipping file.\n");
            free(colourtable);
            return -1;
        }
    }

    /* Seek to the start of the image data */
    fseek(src_fptr, header.offset, SEEK_SET);
    
    /* Allocate memory for bitmap data */
    if ((dest_bitmapdata->bitmap = (char*)malloc(infoheader.height * infoheader.width * sizeof(char))) == NULL) {
        printf(" ERROR: Memory allocation failed. Exiting.\n");
        exit(-1);
    }

    /* Read the image */
    dest_bitmapdata->height = infoheader.height;
    dest_bitmapdata->width = infoheader.width;
    padding = 4 - (infoheader.width % 4);
    padding = (padding == 4)? 0 : padding;
    
    for (i = 0; i < infoheader.height; i++) {
        height_i = infoheader.height-i;
        row_index = (height_i-1)*infoheader.width;
        for (j = 0; j < infoheader.width; j++) {
            if (fread(&colour_index, sizeof(unsigned char), 1, src_fptr) != 1) {
                fprintf(stderr, "\n WARNING: Image read failed. Skipping file.\n");
                free(colourtable);
                return -1;
            }
            if (colourtable[colour_index].r == 0 &&
                colourtable[colour_index].g == 0 &&
                colourtable[colour_index].b == 0) {
                    dest_bitmapdata->bitmap[row_index+j] = BLACK;
                    if(*max_width < j+1) *max_width = j+1;
                    if(*max_height < height_i) *max_height = height_i;
            } else if (colourtable[colour_index].r == 255 &&
                colourtable[colour_index].g == 255 &&
                colourtable[colour_index].b == 255) {
                    dest_bitmapdata->bitmap[row_index+j] = WHITE;
                    if(*max_width < j+1) *max_width = j+1;
                    if(*max_height < height_i) *max_height = height_i;
            } else {dest_bitmapdata->bitmap[row_index+j] = OTHER;
                }
        } /* j */    
        for (j = 0; j < padding; j++) {
            if (fread(&colour_index, sizeof(unsigned char), 1, src_fptr) != 1) {
                fprintf(stderr, "\n WARNING: Image read failed. Skipping file.\n");
                free(colourtable);
                return -1;
            }
        } /* j */
    } /* i */
    
    /* Clean before leaving, then finish */
    free(colourtable);
    fprintf(stderr, " completed.\n");
    return 0;
}

/* 
 * @ Write sprite to file @
 *  
 * Take all sprite data.
 * Write to fptr C code for the sprite.
 * Always returns 0.
 * */
int write_sprite_to_file(
    FILE *fptr,
    char* sprite_name,
    char** costumes_name_table,
    int n_costumes,
    BITMAP_DATA *bitmapdata_arr,
    int ncol,
    int nrow)
{            
    char str_tmp[1024];
    char costume_name_ptr[1024];
    int row, col, byte_nrow, costume, sprite_size;
    int width;
    int height;
    int byte_row_index, bits_left, height_left, byte_tmp, byte_index, bit_in_byte;
    
    
    sprintf(str_tmp,"/***  Sprite %s  ***/\n\n", sprite_name);
    fwrite(str_tmp, 1, strlen(str_tmp), fptr);
        
    byte_nrow = (nrow+7) / 8;
    sprite_size = ncol * byte_nrow;

    for(costume = 0; costume < n_costumes; costume++) {
        
        height = bitmapdata_arr[costume].height;    /* Height of bitmap */
        width = bitmapdata_arr[costume].width;        /* Width of bitmap */
    
        if (strcmp(costumes_name_table[0],"+") == 0) {
            strcpy(costume_name_ptr, sprite_name);
        } else {
            sprintf(costume_name_ptr,"%s_%s",
                sprite_name,
                costumes_name_table[costume]);
        }    
        
        /* Filling black costume */
        
        sprintf(str_tmp,
            "uint8_t %s_sprite_black_costume[%d] = {",
            costume_name_ptr,
            sprite_size);
        fwrite(str_tmp, 1, strlen(str_tmp), fptr);
        
        height_left = height;
        for(row = 0; row < byte_nrow; row++) {
            byte_row_index = row * width * 8;
            bits_left = (height_left < 8)? height_left : 8;
            for(col = 0; col < ncol; col++) {
                
                if(width <= col) break;
                byte_tmp = 0;
                byte_index = byte_row_index + col;
                for(bit_in_byte = 0; bit_in_byte < bits_left; bit_in_byte++) {
                    byte_tmp = byte_tmp | \
                        ((bitmapdata_arr[costume].bitmap[byte_index] == BLACK) << bit_in_byte);
                    byte_index += width;    
                }    /* bit_in_byte */
                sprintf(str_tmp, "%d,",byte_tmp);
                fwrite(str_tmp, 1, strlen(str_tmp), fptr);
            }    /* col */
            height_left -= 8;
            if(height_left <= 0) break;
        } /* row */
        fseek(fptr,-1,SEEK_CUR);
        fwrite("};\n", 1, 3, fptr);
        
        /* Filling white costume */
        
        sprintf(str_tmp,
            "uint8_t %s_sprite_white_costume[%d] = {",
            costume_name_ptr,
            sprite_size);
        fwrite(str_tmp, 1, strlen(str_tmp), fptr);
        
        height_left = height;
        for(row = 0; row < byte_nrow; row++) {
            byte_row_index = row * width * 8;
            bits_left = (height_left < 8)? height_left : 8;
            for(col = 0; col < ncol; col++) {
                
                if(width <= col) break;
                byte_tmp = 0;
                byte_index = byte_row_index + col;
                for(bit_in_byte = 0; bit_in_byte < bits_left; bit_in_byte++) {
                    byte_tmp = byte_tmp | \
                        ((bitmapdata_arr[costume].bitmap[byte_index] == WHITE) << bit_in_byte);
                    byte_index += width;
                }    /* bit_in_byte */
                sprintf(str_tmp, "%d,",byte_tmp);
                fwrite(str_tmp, 1, strlen(str_tmp), fptr);
            }    /* col */
            height_left -= 8;
            if(height_left <= 0) break;
        } /* row */
        fseek(fptr,-1,SEEK_CUR);
        fwrite("};\n\n", 1, 4, fptr);
    }
    
    sprintf(str_tmp,"uint8_t *%s_sprite_black_costumes_bank[%d] = {\n", 
        sprite_name, n_costumes);
    fwrite(str_tmp, 1, strlen(str_tmp), fptr);
    for(costume = 0; costume < n_costumes; costume++) {    
        
        if (strcmp(costumes_name_table[0],"+") == 0) {
            sprintf(str_tmp,"    (uint8_t*)(&%s_sprite_black_costume),\n", 
                sprite_name);
        } else {
            sprintf(str_tmp,"    (uint8_t*)(&%s_%s_sprite_black_costume),\n", 
                sprite_name,
                costumes_name_table[costume]);        
        }
        
        fwrite(str_tmp, 1, strlen(str_tmp), fptr);
    }
    fseek(fptr,-2,SEEK_CUR);
    fwrite("\n};\n", 1, 4, fptr);
    
    sprintf(str_tmp,"uint8_t *%s_sprite_white_costumes_bank[%d] = {\n", 
        sprite_name, n_costumes);
    fwrite(str_tmp, 1, strlen(str_tmp), fptr);
    for(costume = 0; costume < n_costumes; costume++) {
        
        if (strcmp(costumes_name_table[0],"+") == 0) {
            sprintf(str_tmp,"    (uint8_t*)(&%s_sprite_white_costume),\n", 
                sprite_name);
        } else {
            sprintf(str_tmp,"    (uint8_t*)(&%s_%s_sprite_white_costume),\n", 
                sprite_name,
                costumes_name_table[costume]);        
        }
        
        fwrite(str_tmp, 1, strlen(str_tmp), fptr);
    }
    fseek(fptr,-2,SEEK_CUR);
    fwrite("\n};\n\n", 1, 5, fptr);
    
    
    /* Change to one function with switch inside?!! But it's harder... */
    sprintf(str_tmp,"void init_%s_sprite(Sprite *sprite)\n{\n", 
        sprite_name);
    fwrite(str_tmp, 1, strlen(str_tmp), fptr);    
    
    sprintf(str_tmp,"    sprite->black_costumes_bank = %s_sprite_black_costumes_bank;\n", 
        sprite_name);
    fwrite(str_tmp, 1, strlen(str_tmp), fptr);    
    sprintf(str_tmp,"    sprite->white_costumes_bank = %s_sprite_white_costumes_bank;\n\n", 
        sprite_name);
    fwrite(str_tmp, 1, strlen(str_tmp), fptr);
    
    sprintf(str_tmp,"    sprite->width = %d; sprite->height = %d;\n", 
        ncol, byte_nrow);
    fwrite(str_tmp, 1, strlen(str_tmp), fptr);
    
    sprintf(str_tmp,"    sprite->n_costumes = %d;\n}\n\n", 
        n_costumes);
    fwrite(str_tmp, 1, strlen(str_tmp), fptr);
    
    if (n_costumes > 1) {        
        for(costume = 0; costume < n_costumes; costume++) {    
        
            /* Change to define?!! */
            sprintf(str_tmp,"const uint32_t %s_%s = %d;\n", 
                sprite_name,
                costumes_name_table[costume],
                costume);
    
            fwrite(str_tmp, 1, strlen(str_tmp), fptr);
        }
        fwrite("\n", 1, 1, fptr);
    }
    
    return 0;
}

/* 
 * @ Check sprite name @
 *  
 * Check format of name.
 * Object-sprite name should have letter first. 
 * Object-costume - doesn't have to.
 * Then up to 50 characters from
 *  'a'-'z', 'A'-'Z', '0'-'9', '_'.
 * Print error if any.
 * Return 0 if check passed, otherwise error code
 * */
int check_name(char* object_name, int is_sprite_name)
{
    int err_code = 0;
    int length = strlen(object_name);
    int char_idx;
    if(length > 50) err_code = 1;
    if(!(
        (object_name[0] >= 'a' && object_name[0] <= 'z') || \
        (object_name[0] >= 'A' && object_name[0] <= 'Z')) && \
        is_sprite_name == TRUE) err_code = 2;
    for (char_idx = 1; char_idx < length; char_idx++) {
        if(!(
            (object_name[char_idx] >= 'a' && object_name[char_idx] <= 'z') || \
            (object_name[char_idx] >= 'A' && object_name[char_idx] <= 'Z') || \
            (object_name[char_idx] >= '0' && object_name[char_idx] <= '9') || \
            (object_name[char_idx] >= '_'))) {err_code = 3; break;}
    }
    
    if (err_code) {
        fprintf(stderr, " WARNING: Name \"%s\" has invalid format. Skipping it. Reason: ", object_name);
        switch (err_code) {
            case 1: fprintf(stderr, "name is too long.\n"); break;
            case 2: fprintf(stderr, "first name character of sprite is not an allowed letter.\n"); break;
            case 3: fprintf(stderr, "some characters in name are not allowed.\n"); break;
            default: fprintf(stderr, "unknown.\n"); break;
        }
    }
    
    return err_code;    
}

/* 
 * @ Create sprite @
 *  
 * Parse object.
 * If file -> create sprite with one costume.
 * If directory -> create sprite with several costumes 
 *  out of BMP files inside.
 * All created sprites are continiously written
 *  to out_fptr C file.
 * Print log to stderr
 * Return 0 on success, otherwise -1
 * */
int create_sprite(char* main_dir_path, struct dirent *object, FILE *out_fptr)
{
    char object_path[1024];
    char file_path[1024];
    int file, ncol, nrow; 
    int height, width;
    int n_files = 0; 
    BITMAP_DATA *bitmapdata_arr;
    char **bmp_files_names;
    char *object_name, *file_name_ptr, *extension;
    DIR *dir;
    FILE *fptr;
    struct dirent *entry;
    
    /* Check object name */
    
    if ((object_name = (char*)calloc(sizeof(char), strlen(object->d_name)+1)) == NULL) {
        printf(" ERROR: Memory allocation failed. Exiting.\n");
        exit(-1);
    }
    strncpy(object_name, object->d_name, strlen(object->d_name));
    
    if (strcmp(object_name, ".") == 0 || strcmp(object_name, "..") == 0)
        return 0;
        
    snprintf(object_path, sizeof(object_path), "%s/%s", main_dir_path, object_name);
    
    /* Read data from source depending on object type */
    if (object->d_type == DT_DIR) { 
        int failed_files = 0;   
            
        if(check_name(object_name, TRUE))    return -1;
        
        /* Opening directory object */ 
        if (!(dir = opendir(object_path))) {
            fprintf(stderr, " WARNING: Unable to open directory \"%s\"\n", object_path);
            return -1;
        } else {
            fprintf(stderr, "Opened directory \"%s\"\n", object_path);
        }
        
        /* Scanning through dir, counting files */
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type != DT_DIR) n_files++;
        }
        rewinddir(dir);
        
        /* Reading all files in directory */
    
        if ((bitmapdata_arr = (BITMAP_DATA*)malloc(n_files*sizeof(BITMAP_DATA))) == NULL) {
            printf(" ERROR: Memory allocation failed. Exiting.\n");
            exit(-1);
        }
        if ((bmp_files_names = (char**)malloc(n_files*sizeof(char*))) == NULL){
            printf(" ERROR: Memory allocation failed. Exiting.\n");
            exit(-1);
        }
        ncol = nrow = 0;

        for (file = 0; file < n_files; file++) {
            
            /* Skip all directories */
            /* Add other entry types ?!! */
            do {
                if ((entry = readdir(dir)) == NULL) {
                    printf(" WARNING: Directory \"%s\" finished unexpectedly. Skipping it.\n", object_name);
                    return -1;
                }
            } while(entry->d_type == DT_DIR);
            
            snprintf(file_path, sizeof(file_path), "%s/%s", object_path, entry->d_name);

            if ((bmp_files_names[file - failed_files] = (char*)calloc(sizeof(char), strlen(entry->d_name)+1)) == NULL) {
                printf(" ERROR: Memory allocation failed. Exiting.\n");
                exit(-1);
            }
            file_name_ptr = bmp_files_names[file - failed_files];
            strcpy(file_name_ptr, entry->d_name);
            
            extension = (file_name_ptr+strlen(file_name_ptr)-4);
            if(strncmp(extension, ".bmp", 4) == 0) {
                file_name_ptr[strlen(file_name_ptr)-4] = '\0';
            }
            
            /* Check entry name */
            if(check_name(file_name_ptr, FALSE)) {
                failed_files++;
                continue;
            }
            
            /* Open source file */
            if ((fptr = fopen(file_path, "r")) == NULL) {
                fprintf(stderr, " WARNING: Unable to open BMP file \"%s\". Skipping it.\n", file_path);
                failed_files++;
                continue;
            } else {
                fprintf(stderr, "Opened file \"%s\"...", file_path);
            }
            
            if (read_bmp_file(fptr, &bitmapdata_arr[file - failed_files], &width, &height)) {
                failed_files++;
            } else {
                if(ncol < width) ncol = width;
                if(nrow < height) nrow = height;
            }
            
            fclose(fptr);
        }
        n_files -= failed_files;
            
        closedir(dir);
        fprintf(stderr, "Closed directory \"%s\"\n\n", object_path);    
        
    } else {
        n_files = 1;
        
        if ((bitmapdata_arr = (BITMAP_DATA*)malloc(sizeof(BITMAP_DATA))) == NULL) {
            printf(" ERROR: Memory allocation failed. Exiting.\n");
            exit(-1);
        }
        if ((bmp_files_names = (char**)malloc(sizeof(char*))) == NULL) {
            printf(" ERROR: Memory allocation failed. Exiting.\n");
            exit(-1);
        }
        ncol = nrow = 0;
        
        /* Open source file */
        if ((fptr = fopen(object_path, "r")) == NULL) {
            fprintf(stderr, " WARNING: Unable to open BMP file \"%s\". Skipping it.\n", object_path);
            return -1;
        } else {
            fprintf(stderr, "Opened file \"%s\"...", object_path);
        }
        
        if ((bmp_files_names[0] = (char*)calloc(sizeof(char), 2)) == NULL) {
            printf(" ERROR: Memory allocation failed. Exiting.\n");
            exit(-1);
        }
        strcpy(bmp_files_names[0], "+");
                
        object_name[strlen(object_name)-4] = '\0';
                
        if(check_name(object_name,TRUE))    return -1;
        
        if (read_bmp_file(fptr, &bitmapdata_arr[0], &ncol, &nrow))
            return -1;
            
        fprintf(stderr, "\n");
        
        fclose(fptr);                    
    }
    
    write_sprite_to_file(out_fptr, object_name, bmp_files_names,
        n_files, bitmapdata_arr, ncol, nrow);
    
    free(object_name);
    for(file = 0; file < n_files; file++) {
        free(bitmapdata_arr[file].bitmap);
        free(bmp_files_names[file]);
    }
    free(bitmapdata_arr);
    free(bmp_files_names);
    
    return 0;
}

int main(int argc,char **argv)
{
    char *dest_file_name;
    DIR *dir;
    FILE *out_fptr;
    struct dirent *object;    

    /* Check arguments */
    if (argc < 2) {
        fprintf(stderr, "Usage: %s dir_path [output_file_path]\n", argv[0]);
        exit(-1);
    }
    
    fprintf(stderr, "@@@ BMP to C code converter @@@\n Version 1.0");
    
    /* Open source dir */
    if (!(dir = opendir(argv[1]))) {
        fprintf(stderr, " ERROR: Unable to open directory \"%s\"\n", argv[1]);
        exit(-1);
    }
    
    /* Write output file name */
    if (argc >= 3) { 
        if ((dest_file_name = (char*)calloc(sizeof(char), strlen(argv[2])+3)) == NULL) {
            printf(" ERROR: Memory allocation failed. Exiting.\n");
            exit(-1);
        }
        strncpy(dest_file_name, argv[2], strlen(argv[2]));        
    } else {
        if ((dest_file_name = (char*)calloc(sizeof(char), 15)) == NULL) {
            printf(" ERROR: Memory allocation failed. Exiting.\n");
            exit(-1);
        }
        strncpy(dest_file_name, "sprite_table.c", 14);
    }
    
    /* Open output file */
    if ((out_fptr = fopen(dest_file_name, "w")) == NULL) {
        fprintf(stderr, " ERROR: Unable to open C file \"%s\"\n", dest_file_name);
        exit(-1);
    } else {
        fprintf(stderr, "Writing to \"%s\".\n\n", dest_file_name);
    }        
    
    /* Write preambule and typedef struct */
    fwrite("/*************************************************************/\n", 1, 64, out_fptr);
    fwrite("/** This sprite collection file was generated automatically **/\n", 1, 64, out_fptr);
    fwrite("/**  by using BMP to C code converter written in 2019 by ...**/\n", 1, 64, out_fptr);
    fwrite("/**  Ilya Levitsky                                          **/\n", 1, 64, out_fptr);
    fwrite("/*************************************************************/\n\n", 1, 65, out_fptr);
    
    fwrite("#include <stdint.h>\n", 1, 20, out_fptr);
    fwrite("#include \"oled_driver.h\"\n\n", 1, 26, out_fptr);
    
/*    
    fwrite("typedef struct {\n    ", 1, 21, out_fptr);
    fwrite("uint32_t width;\n    ", 1, 20, out_fptr);
    fwrite("uint32_t height;\n    ", 1, 21, out_fptr);
    fwrite("uint8_t **black_costumes_bank;\n    ", 1, 35, out_fptr);
    fwrite("uint8_t **white_costumes_bank;\n    ", 1, 35, out_fptr);
    fwrite("uint32_t n_costumes;\n", 1, 21, out_fptr);
    fwrite("} Sprite;\n\n", 1, 11, out_fptr);
    * */
    
    /* Scanning through main dir */
    while ((object = readdir(dir)) != NULL) {
        create_sprite(argv[1], object, out_fptr);
    }
    
    /* Clean before leaving */
    free(dest_file_name);
    fclose(out_fptr);
    
    fprintf(stderr, "Done!\n");
    
    return 0;
}

/*
   Read a possibly byte swapped unsigned short integer
*/
int ReadUShort(FILE *fptr,short unsigned *n,int swap)
{
   unsigned char *cptr,tmp;

   if (fread(n,2,1,fptr) != 1)
      return(FALSE);
   if (swap) {
      cptr = (unsigned char *)n;
      tmp = cptr[0];
      cptr[0] = cptr[1];
      cptr[1] =tmp;
   }
   return(TRUE);
}

/*
   Read a possibly byte swapped unsigned integer
*/
int ReadUInt(FILE *fptr,unsigned int *n,int swap)
{
   unsigned char *cptr,tmp;

   if (fread(n,4,1,fptr) != 1)
      return(FALSE);
   if (swap) {
      cptr = (unsigned char *)n;
      tmp = cptr[0];
      cptr[0] = cptr[3];
      cptr[3] = tmp;
      tmp = cptr[1];
      cptr[1] = cptr[2];
      cptr[2] = tmp;
   }
   return(TRUE);
}
