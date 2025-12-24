/** 
//todo:
先Initial
inPRINT_Buffer_PutGraphic call CTOS_PrinterBufferBMPPic，把bmp檔案放到buffer 
inPRINT_Buffer_OutPut
 to solve: 無法確認./fs_data 路徑正確與否
**/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <ctosapi.h>

#define d_FILE_READ   0x0001   // 或類似

#include "Trans.h"
#include "print.h"

int inPRINT_Buffer_Initial(unsigned char *uszBuffer, int inYLength, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle)
{
	char		szDebugMsg[100 + 1];
	unsigned short	usRetVal = 0x00;

	memset(uszBuffer, 0x00, PB_CANVAS_X_SIZE * 8 * inYLength);
	memset(srFont_Attrib, 0x00, sizeof(FONT_ATTRIB));
	memset(srBhandle, 0x00, sizeof(BufferHandle));
	
	srFont_Attrib->X_Zoom = 1;		/* 1, it means normal size, and 2 means double size. 0 means print nothing . */
	srFont_Attrib->Y_Zoom = 1;		/* 1, it means normal size, and 2 means double size. 0 means print nothing . */
	srFont_Attrib->X_Space = 0;		/* The space in dot to insert between each character in x coordinate. */
	srFont_Attrib->Y_Space = 0;		/* The space in dot to insert between each character in y coordinate. */
	srFont_Attrib->FontSize = _PRT_HEIGHT_;
	
	srBhandle->inXcurrent = 0;
	srBhandle->inYcurrent = 0;
	srBhandle->inYcurrentMAX = 0;
	srBhandle->inYcover = 0;
	srBhandle->inXbound = PB_CANVAS_X_SIZE;	/* 紙張最寬到哪裡 */
	srBhandle->inYbound = 8 * inYLength;	/* buffer最底部宣告到哪裡 */
	
	/* 這隻是DefaultBuffer用的Initial，QRcode、Barcode會用到，一定要call */
	CTOS_PrinterBufferEnable();
	
	/* 這隻是字串用的Initial */
	usRetVal = CTOS_PrinterBufferInit(uszBuffer, srBhandle->inYbound);
	
	if (usRetVal != d_OK)
	{		
		return (VS_ERROR);
	}
	
	CTOS_PrinterBufferSelectActiveAddress(uszBuffer);	
	return (VS_SUCCESS);
}
int inPRINT_Buffer_PutGraphic(unsigned char *uszFilename, unsigned char* uszBuffer1, BufferHandle *srBhandle, int inGraphicYLength, int inCover)
{
	int		inRetVal;
	char		szDebugMsg[100 + 1];
        unsigned short	usReturnValue;
		
	/* 輸入高度不合法 */
	if (inGraphicYLength <= 0)
	{	
		return (-66);
	}
	if ((srBhandle->inYcurrent + inGraphicYLength) > srBhandle->inYbound )
	{
//		do
//		{
//			inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, srBhandle);
//		} while (inRetVal != VS_SUCCESS);
			
		memset(uszBuffer1, 0x00, srBhandle->inXbound * srBhandle->inYbound);

		srBhandle->inXcurrent = 0;
		srBhandle->inYcurrent = 0;
	}
	
	/* 圖單獨印，所以X位置直接從0 */
	usReturnValue = CTOS_PrinterBufferBMPPic(NULL, 0, srBhandle->inYcurrent, uszFilename);

        if (usReturnValue == d_OK)
        {
		srBhandle->inXcurrent = 0;
		/* 2016/3/21 下午 12:05加入的feature，為了做出以圖片為模板的帳單 */
		if (inCover == _COVER_)
		{
			/* 當底圖handle不下移，但紀錄最低印到哪 */
			srBhandle->inYcover += srBhandle->inYcurrent + inGraphicYLength;
		}
		else
		{
			/* APPEND handle下移 */
			srBhandle->inYcurrent += inGraphicYLength;
		}	
                return (VS_SUCCESS);
        }
        else
        {
                return (usReturnValue);
        }
}
/*
USHORT inCheckPathOK(const char *szPath)
{
    USHORT usRet;
    USHORT usHandle;

    usRet = CTOS_FileOpen(
                (BYTE *)szPath,   // cast 是必要的（CTOS API 設計）
                d_FILE_READ,      // ⚠ 用你 SDK 裡實際定義的
                &usHandle
            );

    if (usRet == d_OK) {
        CTOS_FileClose(usHandle);
    }

    return usRet;
}
*/
int inPRINT_Buffer_PutIn(char* szString, int inFontSize, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle, int inNextLine, unsigned char uszPrintPosition)
{
	int			inRetVal;		/* 若回傳沒紙要重印 */
	
	/* 設定列印字型大小 */
	srFont_Attrib->FontSize = inFontSize;
	inRetVal = inPRINT_Buffer_OutPut(szString, srBhandle);
	
        //inRetVal = inPRINT_Buffer_Sync_UnderLine(szString, uszBuffer, srFont_Attrib, srBhandle, inNextLine, uszPrintPosition, 0);
	
	
	return (inRetVal);	
}
int inPRINT_Buffer_OutPut(unsigned char *uszBuffer, BufferHandle *srBhandle)
{
	char		szTemplate[44 + 1];
	unsigned short	usReturnValue;
	
	/* Q:為什麼要除以8 A:列印的高度以8為單位 Ex:12x24的字需用3行來印(24 = 8 * 3) 12x36需用5行來印(36 < 40 = 8 * 5 )*/
	if (srBhandle->inYcurrent >= srBhandle->inYcover)
		usReturnValue = CTOS_PrinterBufferOutput(uszBuffer, ((srBhandle->inYcurrent) / 8 + 1));
	else
		usReturnValue = CTOS_PrinterBufferOutput(uszBuffer, ((srBhandle->inYcover) / 8) + 1);
        return usReturnValue;
	if (usReturnValue == d_OK)
	{
        	memset(uszBuffer, 0x00, srBhandle->inXbound * srBhandle->inYbound);
		srBhandle->inXcurrent = 0;
		srBhandle->inYcurrent = 0;
		srBhandle->inYcurrentMAX = 0;
		
		return (VS_SUCCESS);
	}
	else
	{
		if (usReturnValue == d_PRINTER_PAPER_OUT)
		{
//			/* 印表機缺紙請裝紙 */
//			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
//			inDISP_PutGraphic(_ERR_PRINT_PAPER_OUT_, 0, _COORDINATE_Y_LINE_8_6_);
//
//			inDISP_BEEP(1, 0);
//			inDISP_Wait(1000);
//
//			if (ginDebug == VS_TRUE)
//			{
//				inLogPrintf(AT, szTemplate);
//				inLogPrintf(AT, "inPRINT_Buffer_OutPut() END!");
//			}

			return (VS_PRINTER_PAPER_OUT);
		}
		else if (usReturnValue == d_PRINTER_HEAD_OVERHEAT)
		{
//			memset(szTemplate, 0x00, sizeof(szTemplate));	
//			if (usReturnValue == d_PRINTER_HEAD_OVERHEAT)
//			{
//				sprintf(szTemplate, "印表機頭過熱");	/* 錯誤代碼 */
//			}
//			else if (usReturnValue == d_PRINTER_MOTOR_OVERHEAT)
//			{
//				sprintf(szTemplate, "印表機馬達過熱");	/* 錯誤代碼 */
//			}
//
//			/* 印表機過熱 */
//			DISPLAY_OBJECT	srDispMsgObj;
//			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
//			strcpy(srDispMsgObj.szDispPic1Name, _ERR_PRINT_);
//			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
//			srDispMsgObj.inMsgType = _ENTER_KEY_MSG_;
//			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
//			strcpy(srDispMsgObj.szErrMsg1, szTemplate);
//			srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
//			srDispMsgObj.inBeepTimes = 1;
//			srDispMsgObj.inBeepInterval = 0;
//			
//			inDISP_Msg_BMP(&srDispMsgObj);
//
//			if (ginDebug == VS_TRUE)
//			{
//				inLogPrintf(AT, szTemplate);
//				inLogPrintf(AT, "inPRINT_Buffer_OutPut() END!");
//			}

			return (VS_PRINTER_OVER_HEAT);
		}
		else
		{
//			memset(szTemplate, 0x00, sizeof(szTemplate));	
//			sprintf(szTemplate, "代碼：0x%04X", usReturnValue);	/* 錯誤代碼 */
//
//			/* 印表機錯誤 */
//			DISPLAY_OBJECT	srDispMsgObj;
//			memset(&srDispMsgObj, 0x00, sizeof(srDispMsgObj));
//			strcpy(srDispMsgObj.szDispPic1Name, _ERR_PRINT_);
//			srDispMsgObj.inDispPic1YPosition = _COORDINATE_Y_LINE_8_6_;
//			srDispMsgObj.inMsgType = _ENTER_KEY_MSG_;
//			srDispMsgObj.inTimeout = _EDC_TIMEOUT_;
//			strcpy(srDispMsgObj.szErrMsg1, szTemplate);
//			srDispMsgObj.inErrMsg1Line = _LINE_8_5_;
//			srDispMsgObj.inBeepTimes = 1;
//			srDispMsgObj.inBeepInterval = 0;
//			
//			inDISP_Msg_BMP(&srDispMsgObj);
//
//			if (ginDebug == VS_TRUE)
//			{
//				inLogPrintf(AT, szTemplate);
//				inLogPrintf(AT, "inPRINT_Buffer_OutPut() END!");
//			}
			return (VS_ERROR);
		}

	}
}
int main(int argc,char *argv[])
{
	int			i = 0;
	int			inPrintIndex = 0, inRetVal;
	char			szShort_Receipt_Mode[2 + 1];
	char			szDebugMsg[100 + 1];
	char			szCustomerIndicator[3 + 1] = {0};
	unsigned char		uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
	BufferHandle		srBhandle;
	FONT_ATTRIB		srFont_Attrib;
        TRANSACTION_OBJECT	pobTran;
        //TRANSACTION_OBJECT*     pobTran = malloc(sizeof(*pobTran));
        pobTran.srBRec.inPrintOption = _PRT_CUST_;
        
       
        CTOS_PrinterFontSelectMode(d_FONT_TTF_MODE);
        CTOS_FontTTFSelectFontFile(d_FONT_DEVICE_PRINTER, "ca_default.ttf", 0);
        CTOS_FontTTFSelectStyle(d_FONT_DEVICE_PRINTER, d_FONT_STYLE_NORMAL);
        
        
        CTOS_PrinterPutString("Test 測試");
        
        inPRINT_Buffer_Initial(uszBuffer,_BUFFER_MAX_LINE_,&srFont_Attrib,&srBhandle);
        
        char 	szPrintBuf[100 + 1];
        memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
	BYTE key;
	CTOS_LCDTClearDisplay();
        char* debugMsg = malloc(1024);
        //console  to the  LCD
        //CTOS_LCDTPrintXY(1, 1, );
        sprintf(szPrintBuf, "card no：%s", "test12345");
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, &srFont_Attrib, &srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        printf("%u\n",sizeof(TRANSACTION_OBJECT));
        printf("%u\n",sizeof(pobTran));
        sprintf(debugMsg, "%d", inRetVal);
        CTOS_LCDTPrintXY(1, 1,szPrintBuf );
        CTOS_KBDGet(&key);
	exit(0);
}
