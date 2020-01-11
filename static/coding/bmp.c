#include <stdlib.h>
#include <stdio.h>

typedef unsigned char   BYTE;
typedef unsigned short  uint16;
typedef unsigned long   DWORD;

int main(int argc,char* argv[])
{
  FILE  *infile;
  FILE  *outfile;
  BYTE  Head[54];
  BYTE  *Buff;
  DWORD ImgWidth,ImgHeight,i,j,p;
  BYTE  bt0,bt1,bt2;
  BYTE stuff[4];
  int mt=1,pt0=200,pt1=100,pt2=100;

  if(argc<1)
  {
   printf("requit files\n",argv[0]);
   return -1;
  }
  infile=fopen("004.bmp","rb+");
  ōutfile=fopen("0055.bmp","wb+");

  if(infile==NULL)
  {
   printf("Open 24 bit bitmap file failed!");
   return -2;
  }

  fread(Head,54,1,infile);
  fwrite((BYTE *)Head,1,54,outfile);

  for(j=0;j<54;j++)
  printf("%d %02x ",j,Head[j]);
  ImgWidth =*(Head+18)+(*(Head+19))*256;
  printf("18 %02x 19 %02x ",*(Head+18),*(Head+19));
  ImgHeight=*(Head+22)+(*(Head+23))*256;
    printf("22 %02x 23 %02x ",*(Head+22),*(Head+23));

   p=(ImgWidth*3)%4;

   printf("Image width: %d  height: %d p: %d \n",ImgWidth,ImgHeight,p);
  Buff = (BYTE *)malloc(ImgWidth*ImgHeight*3);

  if(!Buff)
  {
   printf("Malloc memory failed!\n");
   return -3;
  }

  for(i=0;i<ImgHeight;i++)  // read bitmap pixels array to buffer
  {
   for(j=0;j<ImgWidth;j++)
   {
     fread((BYTE *)&bt0,1,1,infile);
  fread((BYTE *)&bt1,1,1,infile);
  fread((BYTE *)&bt2,1,1,infile);
  *(Buff+i*ImgWidth*3+j*3+0)= bt0;
     *(Buff+i*ImgWidth*3+j*3+1)= bt1;
  *(Buff+i*ImgWidth*3+j*3+2)= bt2;

   if((bt0<(0xff-pt0))&&(bt1>(0xff-pt1))&&(bt2>(0xff-pt2)))
  {

   bt0=0xff;
   bt1=0xff;
   bt2=0xff;
  }

  fwrite((BYTE *)&bt0,1,1,outfile);
  fwrite((BYTE *)&bt1,1,1,outfile);
  fwrite((BYTE *)&bt2,1,1,outfile);
   }
   if(p!=0) fread(stuff,1,(4-p),infile);
  }

  fclose(infile);
return 0;

}
