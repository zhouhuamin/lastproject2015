#ifndef IDDETECT_H_
#define IDDETECT_H_


//箱号所在区域?
typedef struct Region
{
	int x;
	int y;
	int width;
	int height;
	Region()
	{
		x = 0;
		y = 0;
		width = 0;
		height = 0;
	}
}Region;
//红色、黄色、蓝色、白色、灰色、绿色、其他颜色
enum Color {red, yellow, blue, white, gray, green, other};
//箱号排列方式，横排（一排、二排、三排、四排），竖排（一竖、二竖、三竖）
enum AlignType {H, T};//H:horizontal 横排  T：tandem 纵排
typedef struct Align
{
	AlignType Atype;//H:横排
	int count;//排数
	Align()
	{
		Atype = H;
		count = 0;
	}
}Align;
typedef struct ContainerID
{
	unsigned char ID[12];//4位字母+6位箱号+1位校验码+终止符
	unsigned char Type[5];//类型
	Region IDreg;////箱号区域
	Region Typereg;//箱型区域
	Color color;//颜色
	Align ali;//排列方式
	float accuracy;//识别精度
}ContainerID;

extern "C" int  PathReadCode(char *ImagePath,ContainerID *code);//接口1
extern "C" int  CReadCode(unsigned char *Image, int width, int height, ContainerID *code);//接口2
extern "C" int  BReadCode(unsigned char * buff, int width, int height, ContainerID *code);//接口3
#endif