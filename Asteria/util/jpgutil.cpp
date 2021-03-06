#include "jpgutil.h"

JpgUtil::JpgUtil() {

}

void JpgUtil::convertYuyv422(unsigned char * buffer, const unsigned long insize, std::vector<unsigned char> &decodedImage) {

    // Pointer to data in input image buffer
    unsigned char * pBuf = buffer;

    unsigned int pix = 0;

    // Pixels are encoded in groups of four bytes
    for(unsigned int p=0; p<insize; p += 4) {

        char y1 = *(pBuf++);
        char cr = *(pBuf++);
        char y2 = *(pBuf++);
        char cb = *(pBuf++);

        // Discard the colour information, or use it somehow?
        decodedImage[pix++] = y1;
        decodedImage[pix++] = y2;
    }
}

void JpgUtil::readJpeg(unsigned char * buffer, const unsigned long insize, std::vector<unsigned char> &decodedImage) {

    unsigned char r, g, b;
    int width, height;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);

    // Read JPEG from file
//    const char * Name;  /* File name */
//    FILE * infile;        /* source file */
//    if ((infile = fopen(Name, "rb")) == NULL) {
//        fprintf(stderr, "can't open %s\n", Name);
//        return 0;
//    }
//    jpeg_stdio_src(&cinfo, infile);
//    fclose(infile);

    // Read JPEG from memory buffer
    jpeg_mem_src(&cinfo, buffer, insize);

    (void) jpeg_read_header(&cinfo, TRUE);
    (void) jpeg_start_decompress(&cinfo);
    width = cinfo.output_width;
    height = cinfo.output_height;

    int row_stride = width * cinfo.output_components;
    // Output row buffer
    JSAMPARRAY pJpegBuffer = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    unsigned int pix = 0;

    while (cinfo.output_scanline < cinfo.output_height) {
        (void) jpeg_read_scanlines(&cinfo, pJpegBuffer, 1);
        for (int x = 0; x < width; x++) {

            r = pJpegBuffer[0][cinfo.output_components * x];
            if (cinfo.output_components > 2) {
                g = pJpegBuffer[0][cinfo.output_components * x + 1];
                b = pJpegBuffer[0][cinfo.output_components * x + 2];
            } else {
                g = r;
                b = r;
            }

            // Convert RGB to greyscale
            char grey = (r + g + b)/3;
            decodedImage[pix++] = grey;
        }
    }

    (void) jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
}

void JpgUtil::writeJpeg(std::vector<unsigned char> &image, const unsigned int width, const unsigned int height, char *filename) {

    FILE *outfile = fopen( filename, "wb" );
    if ( !outfile )
    {
        fprintf(stderr, "Error opening output jpeg file %s\n!", filename );
        return;
    }

    int bytes_per_pixel = sizeof(unsigned char);

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    cinfo.err = jpeg_std_error( &jerr );
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, outfile);

    // Setting the parameters of the output file here
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = bytes_per_pixel;
    cinfo.in_color_space = JCS_GRAYSCALE;

    // Default compression parameters, we shouldn't be worried about these
    jpeg_set_defaults( &cinfo );
    /* Now do the compression .. */
    jpeg_start_compress( &cinfo, TRUE );

    // Write one row at a time
    // This is a pointer to one row of image data
    JSAMPROW row_pointer[1];
    while( cinfo.next_scanline < cinfo.image_height )
    {
        row_pointer[0] = &image[ cinfo.next_scanline * cinfo.image_width *  cinfo.input_components / bytes_per_pixel];
        jpeg_write_scanlines( &cinfo, row_pointer, 1 );
    }

    // Clean up after we're done compressing
    jpeg_finish_compress( &cinfo );
    jpeg_destroy_compress( &cinfo );
    fclose( outfile );

    return;
}
