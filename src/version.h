#define FILEVER				1,8,0,0
#define PRODUCTVER			FILEVER
#define STRFILEVER			"1.8.0.0"
#define STRPRODUCTVER		STRFILEVER

#define TSI_VERMAJOR		1
#define TSI_VERMINOR		8
#define TSI_VERMICRO		12
#define TSI_VERBUILD		0
#define TSI_VERDATE		__DATE__

#ifdef _WIN64
#define TSI_PLATFORM		"64 Bit"
#else
#define TSI_PLATFORM		"32 Bit"
#endif
