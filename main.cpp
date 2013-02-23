#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jpeglib.h>
#include <jerror.h>
#include <png.h>
#include <stdint.h>

using namespace std;

class CPicInfo
{
public:
    int Width;
    int Height;
}PicInfo;

class ColorData
{
public:
    double Color[4];  //r,g,b,alpha
    unsigned char Alpha;
    ColorData()
    {
        Color[0]=0;
        Color[1]=0;
        Color[2]=0;
        Color[3]=255;
    }
};

int Delta;

void ReadJpeg2Buff(ColorData* &JpegPixOrigData)
{
    
    ColorData* JpegPixOrigDataPtr;
    jpeg_decompress_struct cinfo;
    jpeg_error_mgr jerr;
    
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    
    FILE *infile;
    unsigned char *buffer;
    int x = 0, y = 0;//, PicHeight, PicWidth;
    
    int i, j;
    
    infile = fopen("a.jpg", "rb");
    
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);
    if(cinfo.output_components != 3)
    {
        cout << "ERR! components != 3" << endl;
        return;
    }
    PicInfo.Width = cinfo.output_width;
    PicInfo.Height = cinfo.output_height;
    
    buffer = new unsigned char[ cinfo.output_width * 3 + 2];
    JpegPixOrigData = new ColorData[ cinfo.output_width * cinfo.output_height+2];
    JpegPixOrigDataPtr = JpegPixOrigData;
    
    while (cinfo.output_scanline < cinfo.output_height)
    {
        jpeg_read_scanlines(&cinfo, &buffer, 1);
        for(j = 0; j < cinfo.output_width*3; JpegPixOrigDataPtr++)
        {
            JpegPixOrigDataPtr->Color[0] = buffer[j];
            j++;
            JpegPixOrigDataPtr->Color[1] = buffer[j];
            j++;
            JpegPixOrigDataPtr->Color[2] = buffer[j];
            j++;
        }
    }
    
}

void WritePng(ColorData* PicData)
{
    int i, j, tmp, pos;
    png_byte color_type;
    png_structp png_ptr;
    png_infop info_ptr; 
    png_bytep* row_pointers;
    png_bytep* RowData;
    
    
    FILE *fp = fopen("a.png", "wb");
    
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    info_ptr = png_create_info_struct(png_ptr);
    
//    setjmp(png_jmpbuf(png_ptr));
    
    png_init_io(png_ptr, fp);
    
    //header
    //8? bit_depth
    png_set_IHDR(png_ptr, info_ptr, PicInfo.Width, PicInfo.Height, 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_write_info(png_ptr, info_ptr);
    
    //write
    row_pointers = (png_bytep*)malloc(PicInfo.Height*8);
    for(i = 0; i < PicInfo.Height; i++)
    {
        row_pointers[i] = (png_bytep)malloc(PicInfo.Width*32);
        for(j = 0; j < PicInfo.Width; j++)
        {
            row_pointers[i][4*j] = (png_byte)(PicData[i*PicInfo.Width+j].Color[0]);
            row_pointers[i][4*j+1] = (png_byte)(PicData[i*PicInfo.Width+j].Color[1]);
            row_pointers[i][4*j+2] = (png_byte)(PicData[i*PicInfo.Width+j].Color[2]);
            row_pointers[i][4*j+3] = (png_byte)(PicData[i*PicInfo.Width+j].Color[3]);
        }
    }
    png_write_image(png_ptr, row_pointers);
    
    png_write_end(png_ptr, NULL);
}

/*
double distanceSquare(ColorData &c1, ColorData &c2)
{
    return ( ((double)c1.Color[0]-c2.Color[0])*((double)c1.Color[0]-c2.Color[0]) + ((double)c1.Color[1]-c2.Color[1])*((double)c1.Color[1]-c2.Color[1]) + ((double)c1.Color[2]-c2.Color[2])*((double)c1.Color[2]-c2.Color[2]) );
}
*/
double distanceSquare(ColorData &c1, ColorData &c2, double u=1)    //c1: sum
{
    return ( ((double)c1.Color[0]/u-c2.Color[0])*((double)c1.Color[0]/u-c2.Color[0]) + ((double)c1.Color[1]/u-c2.Color[1])*((double)c1.Color[1]/u-c2.Color[1]) + ((double)c1.Color[2]/u-c2.Color[2])*((double)c1.Color[2]/u-c2.Color[2]) );
}


void work(ColorData* PicData)
{
    ColorData BgInitColor, BgColor;
    int i, j, count = 0, g;
    double u;
    double u1[3],u2[3];
    for(i = 0; i < PicInfo.Width; i++)
    {
        BgInitColor.Color[0] += PicData[i].Color[0];
        BgInitColor.Color[1] += PicData[i].Color[1];
        BgInitColor.Color[2] += PicData[i].Color[2];
        count++;
    }
//    BgInitColor.Color[0] = BgInitColor.Color[0] / PicInfo.Width;
//    BgInitColor.Color[1] = BgInitColor.Color[1] / PicInfo.Width;
//    BgInitColor.Color[2] = BgInitColor.Color[2] / PicInfo.Width;
    
    for(i = 1; i < PicInfo.Height; i++)
    {
        for(j = 0; j < PicInfo.Width; j++)
        {
            g = i * PicInfo.Width + j;
//            u = count * count;
            if(abs(distanceSquare(BgInitColor, PicData[g], count)) > (Delta+10)*(Delta+10) )
            {
                continue;
            }
            BgInitColor.Color[0] += PicData[g].Color[0];
            BgInitColor.Color[1] += PicData[g].Color[1];
            BgInitColor.Color[2] += PicData[g].Color[2];
            count++;
        }
    }
    
    BgInitColor.Color[0] = BgInitColor.Color[0] / count;
    BgInitColor.Color[1] = BgInitColor.Color[1] / count;
    BgInitColor.Color[2] = BgInitColor.Color[2] / count;
    cout << BgInitColor.Color[0]  << " " << BgInitColor.Color[1] << " " << BgInitColor.Color[2] << endl;
    
    for(i = 0; i < PicInfo.Height; i++)
    {
        for(j = 0; j < PicInfo.Width; j++)
        {
            u = count * count;
            g = i * PicInfo.Width + j;
            if(abs(distanceSquare(BgInitColor, PicData[g])) < Delta*Delta )
            {
                PicData[g].Color[3] = 0;
                PicData[g].Color[0] = 0;
                PicData[g].Color[1] = 0;
                PicData[g].Color[2] = 0;  
                continue;
            }
/*            PicData[g].Color[0] = 0;
            PicData[g].Color[1] = 0;
            PicData[g].Color[2] = 0;*/
            PicData[g].Color[3] = 255;
        }
    }
}



//debug mode
void OutputJpegRawData(ColorData* JpegPixOrigData)
{
    int Tmpa, i, j;
    ofstream JpegRawData("JpegRawData.txt");
    JpegRawData << "Jpeg Raw Data, Debug Only" << endl;
    for(i = 0; i < PicInfo.Height; i++)
    {
        for(j = 0; j < PicInfo.Width; j++)
        {
            Tmpa = i*PicInfo.Width+j;
            JpegRawData << (int)JpegPixOrigData[Tmpa].Color[0] << " " << (int)JpegPixOrigData[Tmpa].Color[1] << " " << (int)JpegPixOrigData[Tmpa].Color[2] << " , ";
        }
        JpegRawData << endl;
    }
}

int main()
{
    cin >> Delta;
    ColorData *jpd;
    ReadJpeg2Buff(jpd);
//    OutputJpegRawData(jpd);
    
    work(jpd);
    WritePng(jpd);
    
    return 0;
}