
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctosapi.h>
#include "Trans.h"
#include "print.h"
#include "PrtMsg.h"
#include "EDC.h"

#define _AP_ROOT_PATH_   "./"
#define _SHORT_RECEIPT_U_   "U"
#define _NCCC_TEXT_LOGO_    "財團法人聯合信用卡處理中心"
#define _PROCESS_           "./fs_data/PROCESS.bmp"
#define _COORDINATE_Y_LINE_8_7_  (_LCD_YSIZE_ / 8) * 6
#define _LCD_YSIZE_   480
#define _PRT_12X30_   0x0C1E
#define _SEEK_BEGIN_      d_SEEK_FROM_BEGINNING
#define _SEEK_CURRENT_    d_SEEK_FROM_CURRENT
#define _SEEK_END_        d_SEEK_FROM_EOF
#define d_SEEK_FROM_BEGINNING   0
#define d_SEEK_FROM_CURRENT     1
#define d_SEEK_FROM_EOF         2
#define _PADDING_RIGHT_         0  /* 靠左右補字元 */
#define _PADDING_LEFT_          1  /* 靠右左補字元 */
#define _PRT_NORMAL_            d_FONT_8x16
#define d_FONT_8x16             0x0810
#define _TIMER_NEXSYS_1_        TIMER_ID_1
#define _TIMER_NEXSYS_2_        TIMER_ID_2
#define _TIMER_NEXSYS_3_        TIMER_ID_3
#define _TIMER_NEXSYS_4_        TIMER_ID_4
#define TIMER_ID_1              0
#define TIMER_ID_2              1
#define TIMER_ID_3              2
#define TIMER_ID_4              3
#define _EDC_TIMEOUT_           -1  /* 由EDC.dat控制 */


static EDC_REC srEDCRec;
int ginBAUL_Index = 0;
int imgHeight     = 0;

//Define Ethernet Data //
char ipAddr[] = "10.105.109.155"; //IP Address
char mask[] = "255.255.254.0"; //Mask
char gateWay[] = "10.105.109.254"; //Gateway IP
char hostIp[] = "10.105.108.105"; //Server IP
char hostPort[] = "5000"; //Server Port

typedef struct {
    int inBankLogoHeight; /* 銀行LOGO高度 */
    int inMerchantLogoHeight; /* 商店LOGO高度 */
    int inTitleNameHeight; /* 表頭圖檔高度 */
    int inSloganHeight; /* 企業標語高度 */
    int inCupLegalHeight; /* CUP警語高度 */
    int inInstHeight; /* 分期警語高度 */
    int inNoticeHeight; /* 商店提示與高度 */
} BMPHeight;
BMPHeight gsrBMPHeight;
BufferArrangeUnderLine gsrBAUL[5];

/*
Function	:inPutGraphic
Date&Time	:2015/6/7 下午 5:09
Describe	:顯示LCD BMP圖檔
 */
int inDISP_PutGraphic(char *szFileName, int inX, int inY) {
    int inY_Modify;
    char szDebugMsg[100 + 1];
    unsigned short usRetVal = 0;
    //	if (ginHalfLCD == VS_TRUE)
    //		inY_Modify = inY / 2;
    //	else
    inY_Modify = inY;

    usRetVal = CTOS_LCDGShowBMPPic(inX, inY_Modify, (BYTE *) szFileName);
    ;
    if (usRetVal == d_OK) {
        printf(" CTOS_LCDGShowBMPPic == d_OK\n");
    } else {
        printf("VS_ERROR\n");
        return (VS_ERROR);
    }
    return (VS_SUCCESS);
}

/*
Function	:inFILE_Read
Date&Time	:2015/8/5 下午 5:12
Describe	:讀檔案，傳入Handle接收的Buffer及讀取的size
 */
int inFILE_Read(unsigned long *ulFileHandle, unsigned char *uszReadData, unsigned long ulReadSize) {
    unsigned short usRetVal; /* 實際讀到的長度 */
    usRetVal = CTOS_FileRead(*ulFileHandle, uszReadData, &ulReadSize);

    if (usRetVal != d_OK) {
        return (VS_ERROR);
    }
    return (VS_SUCCESS);
}

/*
Function	:inFILE_Seek
Date&Time	:2015/8/5 下午 5:12
Describe	:尋找檔案，傳入Handle尋找位置及尋找模式_SEEK_BEGIN_或_SEEK_CURRENT_或_SEEK_END_
 */
int inFILE_Seek(unsigned long ulFileHandle, unsigned long ulOffset, int inSeekMode) {
    unsigned short usRetVal = VS_ERROR;

    if (inSeekMode == _SEEK_BEGIN_ || _SEEK_CURRENT_ || _SEEK_END_) {
        usRetVal = CTOS_FileSeek(ulFileHandle, ulOffset, inSeekMode);
    } else {
        return (VS_ERROR);
    }

    if (usRetVal != d_OK) {
        return (VS_ERROR);
    }
    return (VS_SUCCESS);
}

/*
Function	:inFILE_Close
Date&Time	:2015/8/5 下午 5:12
Describe	:關檔案
 */
int inFILE_Close(unsigned long *ulFileHandle) {
    unsigned short usRetVal;
    /* 關閉檔案Handle */
    usRetVal = CTOS_FileClose(*ulFileHandle);

    if (usRetVal != d_OK) {

        return (VS_ERROR);
    }
    //	else
    //	{
    //		inFile_Open_File_Cnt_Decrease();
    //	}

    *ulFileHandle = VS_HANDLE_NULL;
    return (VS_SUCCESS);
}

/*
Function	:inFILE_OpenReadOnly
Date&Time	:2015/9/16 上午 11:34
Describe	:輸入檔案名稱及Handle，開啟檔案，判斷檔案不存在直接Return不建立新檔
 */
int inFILE_OpenReadOnly(unsigned long *ulFileHandle, unsigned char *uszFileName) {
    unsigned long ulFileSize = 0;
    unsigned short usRetVal;

    if (ulFileHandle == NULL || strlen((char *) uszFileName) == 0) {
        return (VS_ERROR);
    }
    /* 取得檔案大小，得到回傳值來判斷檔案是否存在 */
    usRetVal = CTOS_FileGetSize((BYTE *) uszFileName, &ulFileSize);

    /* 判斷檔案不存在，直接Return Error不建立新檔 */
    if (usRetVal == d_FS_FILE_NOT_FOUND) {
        printf("%40s is not found\n", uszFileName);
        return (VS_ERROR);
    }
    /* 用輸入的檔名開啟檔案 */
    usRetVal = CTOS_FileOpen(uszFileName, d_STORAGE_FLASH, ulFileHandle);

    if (usRetVal != d_OK) {
        inFILE_Close(&(*ulFileHandle));
        return (VS_ERROR);
    }
    return (VS_SUCCESS);
}

/*
Function	:inPRINT_Buffer_Get_Height
Date&Time	:2016/3/16 下午 2:15
Describe	:用來預先獲取圖檔的高度，以便利用
 */
int inPRINT_Buffer_GetHeight(unsigned char* uszFileName, int* inHeight) {
    int i;
    int inCalulateHeight = 0;
    char szAscii[8 + 1];
    char szDebugMsg[100 + 1];
    unsigned char uszHex[4 + 1];
    unsigned char uszHeight[4 + 1];
    unsigned long ulHandle;

    memset(&ulHandle, 0x00, sizeof (ulHandle));
    memset(uszHeight, 0x00, sizeof (uszHeight));
    memset(uszHex, 0x00, sizeof (uszHex));
    memset(szAscii, 0x00, sizeof (szAscii));
    /* 初始為0 若因錯誤而跳出則高度為0 */
    *inHeight = 0;

    if (inFILE_OpenReadOnly(&ulHandle, uszFileName) != VS_SUCCESS) {
        return (VS_ERROR);
    }
    /* 16 in hex = 22 in decimal 圖檔放高度資訊的位置 */
    //參數三傳入0  >>  inSeekMode == _SEEK_BEGIN_
    if (inFILE_Seek(ulHandle, 22, 0) != VS_SUCCESS) {
        printf("inFILE_Seek\n");
        return (VS_ERROR);
    }

    if (inFILE_Read(&ulHandle, uszHeight, 4) != VS_SUCCESS) {
        return (VS_ERROR);
    }

    /* height資料只有四個bytes，因為是Little-Endian，所以要先反過來 */
    for (i = 0; i < 4; i++) {
        memcpy(&uszHex[i], &uszHeight[3 - i], 1);
    }
    /* 16進制轉10進制，以byte為單位，從高位乘上256加到低位 */
    for (i = 0; i < 4; i++) { // x*256 = x <<8(256 == 2^8) 
        //uszHex[i] - 0 是為了把字元強制轉整數?
        inCalulateHeight *= 256;
        inCalulateHeight += uszHex[i] - 0;
    }

    /* 加到變數上 */
    *inHeight = inCalulateHeight;
    /* 關閉檔案 */
    inFILE_Close(&ulHandle);
    return (VS_SUCCESS);
}

/*
Function	:inPRINT_TTF_SetFont_Style
Date&Time	:2022/4/20 上午 11:11
Describe	:語言選擇列印的TTF
 */
int inPRINT_TTF_SetFont_Style(int inFontStyle) {
    char szTemplate[44 + 1];
    unsigned short usRetVal = 0;

    usRetVal = CTOS_FontTTFSelectStyle(d_FONT_DEVICE_PRINTER, inFontStyle);

    if (usRetVal != d_OK) {
        return (VS_ERROR);
    }
    return (VS_SUCCESS);
}

int inPRINT_Buffer_Initial(unsigned char *uszBuffer, int inYLength, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle) {
    char szDebugMsg[100 + 1];
    unsigned short usRetVal = 0x00;

    memset(uszBuffer, 0x00, PB_CANVAS_X_SIZE * 8 * inYLength);
    memset(srFont_Attrib, 0x00, sizeof (FONT_ATTRIB));
    memset(srBhandle, 0x00, sizeof (BufferHandle));

    srFont_Attrib->X_Zoom = 1; /* 1, it means normal size, and 2 means double size. 0 means print nothing . */
    srFont_Attrib->Y_Zoom = 1; /* 1, it means normal size, and 2 means double size. 0 means print nothing . */
    srFont_Attrib->X_Space = 0; /* The space in dot to insert between each character in x coordinate. */
    srFont_Attrib->Y_Space = 0; /* The space in dot to insert between each character in y coordinate. */
    srFont_Attrib->FontSize = _PRT_HEIGHT_;

    srBhandle->inXcurrent = 0;
    srBhandle->inYcurrent = 0;
    srBhandle->inYcurrentMAX = 0;
    srBhandle->inYcover = 0;
    srBhandle->inXbound = PB_CANVAS_X_SIZE; /* 紙張最寬到哪裡 */
    srBhandle->inYbound = 8 * inYLength; /* buffer最底部宣告到哪裡 */

    /* 這隻是DefaultBuffer用的Initial，QRcode、Barcode會用到，一定要call */
    CTOS_PrinterBufferEnable();

    /* 這隻是字串用的Initial */
    usRetVal = CTOS_PrinterBufferInit(uszBuffer, srBhandle->inYbound);

    if (usRetVal != d_OK) {
        return (VS_ERROR);
    }

    CTOS_PrinterBufferSelectActiveAddress(uszBuffer);
    return (VS_SUCCESS);
}

int inPRINT_Buffer_Sync_UnderLine(char* szString, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle, int inNextLine, unsigned char uszPrintPosition, int intXPosition) {
    int i;
    int inMaxYLength = 0; /* 所有中最大的高度 */
    int inYcurrentMAX_Old; /* 印失敗要回復原該行底線 */
    int inFontYLength; /* 字型高度 */
    char szDebugMsg[100 + 1]; /* DebugMsg */
    unsigned short usReturnValue; /* ReturnValue */

    memset(&gsrBAUL[ginBAUL_Index].srBhandle, 0x00, sizeof (BufferHandle));
    memcpy(&gsrBAUL[ginBAUL_Index].srBhandle, srBhandle, sizeof (BufferHandle));

    memset(&gsrBAUL[ginBAUL_Index].szString, 0x00, sizeof (szString));
    memcpy(&gsrBAUL[ginBAUL_Index].szString, szString, strlen(szString));

    memset(&gsrBAUL[ginBAUL_Index].srFont_Attrib, 0x00, sizeof (FONT_ATTRIB));
    memcpy(&gsrBAUL[ginBAUL_Index].srFont_Attrib, srFont_Attrib, sizeof (FONT_ATTRIB));

    if (uszPrintPosition != _PRINT_BY_X_) {
        /* Aligned */
        gsrBAUL[ginBAUL_Index].uszPrintPosition = uszPrintPosition;
    } else {
        /* By x位置 */
        gsrBAUL[ginBAUL_Index].uszPrintPosition = uszPrintPosition;
        gsrBAUL[ginBAUL_Index].intXPosition = intXPosition;
    }
    ginBAUL_Index++;

    if (inNextLine == _CURRENT_LINE_) {
    }/* Last Entry */
    else {
        /* Find Max Y length */
        for (i = 0; i < ginBAUL_Index; i++) {
            inFontYLength = gsrBAUL[i].srFont_Attrib.FontSize % 0x0100;
            if (inMaxYLength < inFontYLength)
                inMaxYLength = inFontYLength;
        }

        /* 紀錄當前行底線 */
        /* 萬一失敗拿來還原用 */
        inYcurrentMAX_Old = srBhandle->inYcurrentMAX;
        /* 該行底線設為該行字型 */
        srBhandle->inYcurrentMAX = srBhandle->inYcurrent + inMaxYLength;

        for (i = 0; i < ginBAUL_Index; i++) {
            inFontYLength = gsrBAUL[i].srFont_Attrib.FontSize % 0x0100;

            /* 改變字體形式用 */
            if (gsrBAUL[i].inFontStyle != _BAUL_FONT_STYLE_UNCHANGE_) {
                if (gsrBAUL[i].inFontStyle == _BAUL_FONT_STYLE_REGULAR_) {
                    inPRINT_TTF_SetFont_Style(_FONT_PRINT_REGULAR_);
                } else if (gsrBAUL[i].inFontStyle == _BAUL_FONT_STYLE_ITALIC_) {
                    inPRINT_TTF_SetFont_Style(_FONT_PRINT_ITALIC_);
                } else if (gsrBAUL[i].inFontStyle == _BAUL_FONT_STYLE_BOLD_) {
                    inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);
                } else if (gsrBAUL[i].inFontStyle == _BAUL_FONT_STYLE_REVERSE_) {
                    inPRINT_TTF_SetFont_Style(_FONT_PRINT_REVERSE_);
                } else if (gsrBAUL[i].inFontStyle == _BAUL_FONT_STYLE_UNDERLINE_) {
                    inPRINT_TTF_SetFont_Style(_FONT_PRINT_UNDERLINE_);
                }
            }

            if (gsrBAUL[i].uszPrintPosition != _PRINT_BY_X_) {
                usReturnValue = CTOS_PrinterBufferPutStringAligned(uszBuffer, srBhandle->inYcurrentMAX - inFontYLength, (unsigned char*) &gsrBAUL[i].szString, &gsrBAUL[i].srFont_Attrib, gsrBAUL[i].uszPrintPosition);
            } else {
                usReturnValue = CTOS_PrinterBufferPutString(uszBuffer, gsrBAUL[i].intXPosition, srBhandle->inYcurrentMAX - inFontYLength, (unsigned char*) &gsrBAUL[i].szString, &gsrBAUL[i].srFont_Attrib);
            }

            if (usReturnValue == d_OK) {
                /* 寫成功 */
            } else {
                /* 寫失敗，拿來還原用 */
                srBhandle->inYcurrentMAX = inYcurrentMAX_Old;

                inPRINT_TTF_SetFont_Style(_FONT_PRINT_BOLD_);

                return (VS_ERROR);
            }
        }

        /* 換下一行 */
        srBhandle->inXcurrent = 0;
        srBhandle->inYcurrent = srBhandle->inYcurrentMAX;

        /* 清空資料暫存 */
        memset(gsrBAUL, 0x00, sizeof (BufferArrangeUnderLine) * ginBAUL_Index);
        ginBAUL_Index = 0;
    }

    return (VS_SUCCESS);
}

int inPRINT_Buffer_PutIn(char* szString, int inFontSize, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle, int inNextLine, unsigned char uszPrintPosition) {
    int inRetVal; /* 若回傳沒紙要重印 */

    /* 設定列印字型大小 */
    srFont_Attrib->FontSize = inFontSize;
    /* 放超過Buffer高度，回傳錯誤(直接假設該行印最大字型，若會超出bound就直接印出來) */
    if ((srBhandle->inYcurrent + _MAX_Y_LENGTH_) > srBhandle->inYbound) {
        /* 多筆明細報表列印做畫面切換 避免誤認當機 */
        if (srBhandle->uszDetailPrint == VS_TRUE) {
            //			inDISP_PutGraphic(_PRT_RECEIPT_, 0, _COORDINATE_Y_LINE_8_7_);	/* 帳單列印中 */
        }
        printf("當放超過高度 直接OutPut並清空Buffer(保險起見)\n");
        /* 當放超過高度 直接OutPut並清空Buffer(保險起見) */
        inRetVal = inPRINT_Buffer_OutPut(uszBuffer, srBhandle);

        /* 多筆明細報表列印做畫面切換 避免誤認當機 */
        if (srBhandle->uszDetailPrint == VS_TRUE) {
            inDISP_PutGraphic(_PROCESS_, 0, _COORDINATE_Y_LINE_8_7_); /* 處理中... */
        }
    }

    inRetVal = inPRINT_Buffer_Sync_UnderLine(szString, uszBuffer, srFont_Attrib, srBhandle, inNextLine, uszPrintPosition, 0);
    return (inRetVal);
}

int inPRINT_Buffer_OutPut(unsigned char *uszBuffer, BufferHandle *srBhandle) {
    char szTemplate[44 + 1];
    unsigned short usReturnValue;
    //printf("PRINT_Buffer_OutPut\n");
    /* Q:為什麼要除以8 A:列印的高度以8為單位 Ex:12x24的字需用3行來印(24 = 8 * 3) 12x36需用5行來印(36 < 40 = 8 * 5 )*/
    if (srBhandle->inYcurrent >= srBhandle->inYcover)
        usReturnValue = CTOS_PrinterBufferOutput(uszBuffer, ((srBhandle->inYcurrent) / 8 + 1));
    else
        usReturnValue = CTOS_PrinterBufferOutput(uszBuffer, ((srBhandle->inYcover) / 8) + 1);
    return usReturnValue;
    if (usReturnValue == d_OK) {
        memset(uszBuffer, 0x00, srBhandle->inXbound * srBhandle->inYbound);
        srBhandle->inXcurrent = 0;
        srBhandle->inYcurrent = 0;
        srBhandle->inYcurrentMAX = 0;

        return (VS_SUCCESS);
    } else {
        if (usReturnValue == d_PRINTER_PAPER_OUT) {
            //			/* 印表機缺紙請裝紙 */
            //			inDISP_Clear_Line(_LINE_8_4_, _LINE_8_8_);
            //			inDISP_PutGraphic(_ERR_PRINT_PAPER_OUT_, 0, _COORDINATE_Y_LINE_8_6_);
            //
            //			inDISP_BEEP(1, 0);
            //			inDISP_Wait(1000);

            return (VS_PRINTER_PAPER_OUT);
        } else if (usReturnValue == d_PRINTER_HEAD_OVERHEAT) {
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

            return (VS_PRINTER_OVER_HEAT);
        } else {
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
            //			inDISP_Msg_BMP(&srDispMsgObj);
            return (VS_ERROR);
        }
    }
}

int inCREDIT_PRINT_MerchantLogo(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, BufferHandle *srBhandle) {
    if (inPRINT_Buffer_PutGraphic((unsigned char*) _MERCHANT_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inMerchantLogoHeight, _APPEND_) != VS_SUCCESS) {
        return (VS_ERROR);
    }
    return (VS_SUCCESS);
}

/*
Function        :inCREDIT_PRINT_MerchantName
Date&Time       :2016/9/7 下午 5:55
Describe        :用來決定要不要印商店表頭
 */
int inCREDIT_PRINT_MerchantName(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, BufferHandle *srBhandle) {
    if (inPRINT_Buffer_PutGraphic((unsigned char*) _NAME_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inTitleNameHeight, _APPEND_) != VS_SUCCESS) {
        return (VS_ERROR);
    }
    return (VS_SUCCESS);
}

int inCREDIT_PRINT_Logo_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle) {
    /* 印Slogan 384*180 */
    //	if (pobTran->srBRec.inPrintOption == _PRT_CUST_)
    //	{
    //		if (inCREDIT_PRINT_MarchantSlogan(pobTran, _NCCC_SLOGAN_PRINT_UP_, uszBuffer, srBhandle) != VS_SUCCESS)
    //			return (VS_ERROR);
    //	}

    //        if (!memcmp(szShort_Receipt_Mode, _SHORT_RECEIPT_U_, strlen(_SHORT_RECEIPT_U_)))
    //        {
    /* 印NCC的LOGO */
    //                if (inPRINT_Buffer_PutIn(_NCCC_TEXT_LOGO_, _PRT_12X30_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_CENTER_) != VS_SUCCESS)
    //                {
    //                        return (VS_ERROR);
    //                }
    //        }
    //        else
    //        {
    //                /* 印NCC的LOGO */

    if (inPRINT_Buffer_PutGraphic((unsigned char*) _BANK_LOGO_, uszBuffer, srBhandle, gsrBMPHeight.inBankLogoHeight, _APPEND_) != VS_SUCCESS) {
        return (VS_ERROR);
    }
    //        }
    //
    //        /* 印商店的LOGO */
    if (inCREDIT_PRINT_MerchantLogo(pobTran, uszBuffer, srBhandle) != VS_SUCCESS) {
        return (VS_ERROR);
    }
    //	
    //	/* 印商店名稱 */
    if (inCREDIT_PRINT_MerchantName(pobTran, uszBuffer, srBhandle) != VS_SUCCESS) {
        return (VS_ERROR);
    }
    return (VS_SUCCESS);
}

int inPRINT_Buffer_PutGraphic(unsigned char *uszFilename, unsigned char* uszBuffer1, BufferHandle *srBhandle, int inGraphicYLength, int inCover) {
    int inRetVal;
    char szDebugMsg[100 + 1];
    unsigned short usReturnValue;

    /* 輸入高度不合法 */
    if (inGraphicYLength <= 0) {
        return (-66);
    }
    if ((srBhandle->inYcurrent + inGraphicYLength) > srBhandle->inYbound) {
        do {
            inRetVal = inPRINT_Buffer_OutPut(uszBuffer1, srBhandle);
        } while (inRetVal != VS_SUCCESS);

        memset(uszBuffer1, 0x00, srBhandle->inXbound * srBhandle->inYbound);

        srBhandle->inXcurrent = 0;
        srBhandle->inYcurrent = 0;
    }
    /* 圖單獨印，所以X位置直接從0 */
    usReturnValue = CTOS_PrinterBufferBMPPic((unsigned char*) uszBuffer1, 0, srBhandle->inYcurrent, uszFilename);

    if (usReturnValue == d_OK) {
        //printf("d_OK\n");
        srBhandle->inXcurrent = 0;
        /* 2016/3/21 下午 12:05加入的feature，為了做出以圖片為模板的帳單 */
        if (inCover == _COVER_) {
            /* 當底圖handle不下移，但紀錄最低印到哪 */
            srBhandle->inYcover += srBhandle->inYcurrent + inGraphicYLength;
        } else {
            /* APPEND handle下移 */
            srBhandle->inYcurrent += inGraphicYLength;
        }
        return (VS_SUCCESS);
    } else {
        printf("%s got usReturnValue is 0x%Xh \n", uszFilename, usReturnValue);
        return (usReturnValue);
    }
}

/*
Function        :inCREDIT_PRINT_TIDMID_ByBuffer
Date&Time       :2015/8/10 上午 10:24
Describe        :列印TID & MID
 */
int inCREDIT_PRINT_Tidmid_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle) {
    int inRetVal;
    char szPrintBuf[84 + 1], szTemplate[42 + 1];

    memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
    memset(szTemplate, 0x00, sizeof (szTemplate));
    int inPrinttype_ByBuffer = 0;
    if (inPrinttype_ByBuffer) {
        /* 直式 */
        /* Get商店代號 */
        //                memset(szTemplate, 0x00, sizeof(szTemplate));
        //                inGetMerchantID(szTemplate);

        /* 列印商店代號 */
        memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
        //                sprintf(szPrintBuf, "商店代號：%s", szTemplate);
        sprintf(szPrintBuf, "商店代號：0001");
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        if (inRetVal != VS_SUCCESS)
            return (VS_ERROR);

        /* Get端末機代號 */
        //                memset(szTemplate, 0x00, sizeof(szTemplate));
        //                inGetTerminalID(szTemplate);

        /* 列印端末機代號 */
        memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
        //                sprintf(szPrintBuf, "端末機代號：%s", szTemplate);
        sprintf(szPrintBuf, "端末機代號：1234");
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        if (inRetVal != VS_SUCCESS)
            return (VS_ERROR);
    } else {
        /* 橫式 */
        //                memset(szTemplate, 0x00, sizeof(szTemplate));
        //                inGetMerchantID(szTemplate);

        /* 列印商店代號 */
        //                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 15, _PADDING_LEFT_);
        sprintf(szPrintBuf, "商店代號 4567");
        //                sprintf(szPrintBuf, "商店代號 %s", szTemplate);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        if (inRetVal != VS_SUCCESS)
            return (VS_ERROR);

        /* Get端末機代號 */
        //                memset(szTemplate, 0x00, sizeof(szTemplate));
        //                inGetTerminalID(szTemplate);

        /* 列印端末機代號 */
        //                inFunc_PAD_ASCII(szTemplate, szTemplate, ' ', 13, _PADDING_LEFT_);
        sprintf(szPrintBuf, "端末機代號 121314151617");
        //                sprintf(szPrintBuf, "端末機代號 %s", szTemplate);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        if (inRetVal != VS_SUCCESS)
            return (VS_ERROR);

        inRetVal = inPRINT_Buffer_PutIn("================================================", _PRT_NORMAL_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        if (inRetVal != VS_SUCCESS)
            return (VS_ERROR);
    }

    return (inRetVal);
}

int inCREDIT_PRINT_Data_ByBuffer(TRANSACTION_OBJECT *pobTran, unsigned char *uszBuffer, FONT_ATTRIB *srFont_Attrib, BufferHandle *srBhandle) {
    int i;
    int inRetVal;
    char szPrintBuf[100 + 1], szPrintBuf1[42 + 1], szPrintBuf2[42 + 1], szTemplate1[42 + 1], szTemplate2[42 + 1];
    char szProductCodeEnable[1 + 1];
    char szStore_Stub_CardNo_Truncate_Enable[2 + 1];
    char szExamBit[10 + 1] = {0};
    char szCustomerIndicator[3 + 1] = {0};
    unsigned char uszChangeLineBit = VS_FALSE;

    memset(szCustomerIndicator, 0x00, sizeof (szCustomerIndicator));
    //inGetCustomIndicator(szCustomerIndicator);

    memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
    memset(szPrintBuf1, 0x00, sizeof (szPrintBuf1));
    memset(szPrintBuf2, 0x00, sizeof (szPrintBuf2));
    memset(szTemplate1, 0x00, sizeof (szTemplate1));
    memset(szTemplate2, 0x00, sizeof (szTemplate2));
    int inPrinttype_ByBuffer = 1; //寫死 跑直式
    if (inPrinttype_ByBuffer) {
        printf("列印直式-------\n");
        /* 直式 */
        /*卡別、卡號*/
        /* 【需求單 - 106349】自有品牌判斷需求 */
        /* unpack電文就存存CardLabel modify by LingHsiung 2020/2/14 上午 11:07 */
        sprintf(szPrintBuf, "卡別　　：%s", pobTran->srBRec.szCardLabel);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
        sprintf(szPrintBuf, "卡號　　：%s", pobTran->srBRec.szPAN);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        /*日期、時間*/
        memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
        sprintf(szPrintBuf, "日期　　：%s", pobTran->srBRec.szDate);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
        sprintf(szPrintBuf, "時間　　：%s", pobTran->srBRec.szTime);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        /*調閱編號、批次號碼 */
        memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
        sprintf(szPrintBuf, "調閱編號：%06ld", pobTran->srBRec.lnOrgInvNum);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        memset(szPrintBuf, 0x00, sizeof (szPrintBuf));
        sprintf(szPrintBuf, "批次號碼：%06ld", pobTran->srBRec.lnBatchNum);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        //                /*交易類別*/
        //                inFunc_GetTransType(pobTran, szPrintBuf1, szTemplate2);
        //                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "交易類別:testType");
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        //                /*授權碼、序號*/
        sprintf(szPrintBuf, "授權碼　：%s", pobTran->srBRec.szAuthCode);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);
        //
        //                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
        sprintf(szPrintBuf, "序號　　：%s", pobTran->srBRec.szRefNo);
        inRetVal = inPRINT_Buffer_PutIn(szPrintBuf, _PRT_DOUBLE_HEIGHT_WIDTH_, uszBuffer, srFont_Attrib, srBhandle, _LAST_ENTRY_, _PRINT_LEFT_);

        //                memset(szPrintBuf, 0x00, sizeof(szPrintBuf));
    }
    return VS_SUCCESS;
}

int inFunc_ShellCommand_System(char *szCommand) {
    int inRetVal;
    char szDebugMsg[100 + 1];
    if (szCommand == NULL) {
        return (VS_ERROR);
    }
    inRetVal = system(szCommand);
    /* 在C 程式裡其中 ret 值要除以256才會得到與shell 傳回相符的值 */
    inRetVal >>= 8;

    if (inRetVal != 0) {
        return (VS_ERROR);
    }
    return (VS_SUCCESS);
}

int inFunc_ls(char* szParameter1, char* szSource) {
    int inRetVal = VS_SUCCESS;
    char szDebugMsg[100 + 1] = {0};
    char szCommand[100 + 1] = {0}; /* Shell Command*/

    /* 組命令 */
    if (strlen(szSource) != 0) {
        memset(szCommand, 0x00, sizeof (szCommand));
        sprintf(szCommand, "ls ");

        if (strlen(szParameter1) != 0) {
            strcat(szCommand, szParameter1);
            strcat(szCommand, " ");
        }

        if (strlen(szSource) != 0) {
            strcat(szCommand, szSource);
        }
    }/* 沒有來源檔 */
    else {
        return (VS_ERROR);
    }
    //printf("szCommand is %s\n",szCommand);
    /* 執行命令 */
    inRetVal = inFunc_ShellCommand_System(szCommand);
    if (inRetVal != VS_SUCCESS) {
        return (inRetVal);
    }

    return (VS_SUCCESS);
}

/*
Function	:inPRINT_Buffer_GetHeightFlow
Date&Time	:2016/3/16 下午 2:48
Describe	:決定要加入幾張圖，不return error是因為當有缺圖，其他圖仍能運作正常
 */
int inPRINT_Buffer_GetHeightFlow(void) {
    int inPreLen = 0;
    int inPathLen = 0;
    char szPath[100 + 1];
    char szFileName[50 + 1];
    /* 初始化結構 */
    memset(&gsrBMPHeight, 0x00, sizeof (BMPHeight));

    inPreLen = strlen("./fs_data/");

    /* 銀行LOGO高度 */
    memset(szPath, 0x00, sizeof (szPath));
    inPathLen = strlen(_BANK_LOGO_);
    memcpy(szPath, _BANK_LOGO_, inPathLen);

    memset(szFileName, 0x00, sizeof (szFileName));
    memcpy(szFileName, &szPath[inPreLen], inPathLen - inPreLen);

    inPRINT_Buffer_GetHeight((unsigned char*) szFileName, &gsrBMPHeight.inBankLogoHeight);

    /* 商店LOGO高度 */
    memset(szPath, 0x00, sizeof (szPath));
    inPathLen = strlen(_MERCHANT_LOGO_);
    memcpy(szPath, _MERCHANT_LOGO_, inPathLen);

    memset(szFileName, 0x00, sizeof (szFileName));
    memcpy(szFileName, &szPath[inPreLen], inPathLen - inPreLen);

    inPRINT_Buffer_GetHeight((unsigned char*) szFileName, &gsrBMPHeight.inMerchantLogoHeight);

    /* 表頭圖檔高度 */
    memset(szPath, 0x00, sizeof (szPath));
    inPathLen = strlen(_NAME_LOGO_);
    memcpy(szPath, _NAME_LOGO_, inPathLen);

    memset(szFileName, 0x00, sizeof (szFileName));
    memcpy(szFileName, &szPath[inPreLen], inPathLen - inPreLen);

    inPRINT_Buffer_GetHeight((unsigned char*) szFileName, &gsrBMPHeight.inTitleNameHeight);

    /* 企業標語高度 */
    memset(szPath, 0x00, sizeof (szPath));
    inPathLen = strlen(_SLOGAN_LOGO_);
    memcpy(szPath, _SLOGAN_LOGO_, inPathLen);

    memset(szFileName, 0x00, sizeof (szFileName));
    memcpy(szFileName, &szPath[inPreLen], inPathLen - inPreLen);

    inPRINT_Buffer_GetHeight((unsigned char*) szFileName, &gsrBMPHeight.inSloganHeight);

    /* CUP警語高度 */
    memset(szPath, 0x00, sizeof (szPath));
    inPathLen = strlen(_CUP_LEGAL_LOGO_);
    memcpy(szPath, _CUP_LEGAL_LOGO_, inPathLen);

    memset(szFileName, 0x00, sizeof (szFileName));
    memcpy(szFileName, &szPath[inPreLen], inPathLen - inPreLen);

    inPRINT_Buffer_GetHeight((unsigned char*) szFileName, &gsrBMPHeight.inCupLegalHeight);

    /* 分期警語高度 */
    memset(szPath, 0x00, sizeof (szPath));
    inPathLen = strlen(_LEGAL_LOGO_);
    memcpy(szPath, _LEGAL_LOGO_, inPathLen);

    memset(szFileName, 0x00, sizeof (szFileName));
    memcpy(szFileName, &szPath[inPreLen], inPathLen - inPreLen);

    inPRINT_Buffer_GetHeight((unsigned char*) szFileName, &gsrBMPHeight.inInstHeight);

    /* 商店提示與高度*/
    memset(szPath, 0x00, sizeof (szPath));
    inPathLen = strlen(_NOTICE_LOGO_);
    memcpy(szPath, _NOTICE_LOGO_, inPathLen);

    memset(szFileName, 0x00, sizeof (szFileName));
    memcpy(szFileName, &szPath[inPreLen], inPathLen - inPreLen);

    inPRINT_Buffer_GetHeight((unsigned char*) szFileName, &gsrBMPHeight.inNoticeHeight);
    return (VS_SUCCESS);
}

/*
Function        :inFunc_Booting_Flow_Print_Image_Initial
Date&Time       :2018/6/5 下午 6:27
Describe        :開機流程列印圖片初始化，為了避免開機更新導致抓不到圖片高度
 */
int inFunc_Booting_Flow_Print_Image_Initial(TRANSACTION_OBJECT *pobTran) {
    /* 若沒下TMS會沒圖片抓高度，所以不判斷成功或失敗 */
    inPRINT_Buffer_GetHeightFlow();
    return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Cofig_Get
Date&Time       :2018/5/15 下午 5:26
Describe        :
 */
int inETHERNET_Cofig_Get(unsigned char uszTag, unsigned char *uszValue, unsigned char *uszLen) {
    char szDebugMsg[100 + 1] = {0};
    unsigned short usRetVal;

    usRetVal = CTOS_EthernetConfigGet(uszTag, uszValue, uszLen);
    if (usRetVal == d_OK) {
        printf("CTOS_EthernetConfigGet successed\n");
    } else {
        printf("CTOS_EthernetConfigGet failed, retVal is %X\n", usRetVal);
        return (VS_ERROR);
    }

    return (VS_SUCCESS);
}

int inSetTermIPAddress(char* szTermIPAddress) {
    memset(srEDCRec.szTermIPAddress, 0x00, sizeof (srEDCRec.szTermIPAddress));
    /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
    if (szTermIPAddress == NULL || strlen(szTermIPAddress) < 0 || strlen(szTermIPAddress) > 15) {
        printf("inSetTermIPAddress failed \n");
        return (VS_ERROR);
    }
    memcpy(&srEDCRec.szTermIPAddress[0], &szTermIPAddress[0], strlen(szTermIPAddress));

    return (VS_SUCCESS);
}

int inSetTermMASKAddress(char* szTermMASKAddress) {
    memset(srEDCRec.szTermMASKAddress, 0x00, sizeof (srEDCRec.szTermMASKAddress));
    /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
    if (szTermMASKAddress == NULL || strlen(szTermMASKAddress) < 0 || strlen(szTermMASKAddress) > 15) {
        printf("inSetTermMASKAddress failed\n");
        return (VS_ERROR);
    }
    memcpy(&srEDCRec.szTermMASKAddress[0], &szTermMASKAddress[0], strlen(szTermMASKAddress));

    return (VS_SUCCESS);
}

int inSetTermGetewayAddress(char* szTermGetewayAddress) {
    memset(srEDCRec.szTermGetewayAddress, 0x00, sizeof (srEDCRec.szTermGetewayAddress));
    /* 傳進的指標 不得為空  長度需大於0 小於規定最大值 */
    if (szTermGetewayAddress == NULL || strlen(szTermGetewayAddress) < 0 || strlen(szTermGetewayAddress) > 15) {
        printf("inSetTermGetewayAddress failed\n");
        return (VS_ERROR);
    }
    memcpy(&srEDCRec.szTermGetewayAddress[0], &szTermGetewayAddress[0], strlen(szTermGetewayAddress));

    return (VS_SUCCESS);
}

int inTimerStart(int inTimerNbr, long lnDelay) {
    CTOS_TimeOutSet(inTimerNbr, lnDelay * 100);

    return (VS_SUCCESS);
}

int inDISP_Timer_Start(int inTimerNumber, long lnDelay) {
    int inEnterTimeout = 0;
    char szEnterTimeout[3 + 1];

    if (lnDelay == _EDC_TIMEOUT_) {
        memset(szEnterTimeout, 0x00, sizeof (szEnterTimeout));
        //inGetEnterTimeout(szEnterTimeout); //to fix
        inEnterTimeout = atoi(szEnterTimeout);
        if (inEnterTimeout != 0) {
            inTimerStart(inTimerNumber, (long) inEnterTimeout);
        } else {
            return (VS_ERROR);
        }
    } else {
        inTimerStart(inTimerNumber, lnDelay);
    }

    return (VS_SUCCESS);
}

int inETHERNET_Open() {
    char szDebugMsg[100 + 1];
    unsigned short usRetVal;

    usRetVal = CTOS_EthernetOpen();
    if (usRetVal == d_OK) {
        //printf("inETHERNET_Open successed\n");
    } else if (usRetVal == d_ETHERNET_ALREADY_OPENED) {
        //printf("usRetVal == d_ETHERNET_ALREADY_OPENED\n");
    } else {
        printf("inETHERNET_Open failed\n");
        return (VS_ERROR);
    }

    return (VS_SUCCESS);
}

int inETHERNET_Cofig_Set(unsigned char uszTag, unsigned char *uszValue, unsigned char uszLen) {
    char szDebugMsg[100 + 1];
    unsigned short usRetVal = 0x00;

    usRetVal = CTOS_EthernetConfigSet(uszTag, uszValue, uszLen);
    if (usRetVal == d_OK) {
        printf("CTOS_EthernetConfigSet successed\n");
    } else {
        printf("CTOS_EthernetConfigSet failed,%s,usRetVal:%X\n",uszValue,usRetVal);
        return (VS_ERROR);
    }

    return (VS_SUCCESS);
}

/*
Function        :inETHERNET_Initial
Date&Time       :2017/7/18 下午 3:37
Describe        :
 */
int inETHERNET_Initial(void) {
    int inRetVal = VS_ERROR;
    char szEDCIPAddress[16 + 1];
    char szSubMask[16 + 1];
    char szGatWay[16 + 1];
    char szConfig[16 + 1];
    char szDHCPMode[2 + 1];
    unsigned short usRetVal = 0x00;
    char szDispMsg[16 + 1] = {0};
    unsigned char uszLen = 0;
    
    inSetTermIPAddress(ipAddr);
    inSetTermMASKAddress(mask);
    inSetTermGetewayAddress(gateWay);
    
    //暫不實現
    //	inDISP_ClearAll();
    //	inFunc_Display_LOGO( 0,  _COORDINATE_Y_LINE_16_2_);				/* 第一層顯示 LOGO */
    //	inDISP_PutGraphic(_MENU_SET_COMM_TITLE_, 0, _COORDINATE_Y_LINE_8_3_);		/* 第三層顯示 <通訊設定> */

    //	inDISP_Timer_Start(_TIMER_NEXSYS_1_, _EDC_TIMEOUT_);

    /* 打開TCP IP */
    /* Portable 機型若沒接上底座再開Ethernet會失敗 */
    /* 這邊要分是否為有底座機型，有底座要先接回才能Initial */
    inRetVal = inETHERNET_Open();

    if (inRetVal == VS_SUCCESS) {
        //printf("inETHERNET_Open  successed\n");
    } else {
        printf("inETHERNET_Open  failed,ret is %d\n",inRetVal);
        return (VS_ERROR);
    }
    /* Set EDC IP Address */
    if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_IP, (unsigned char*) srEDCRec.szTermIPAddress, strlen(srEDCRec.szTermIPAddress)) != VS_SUCCESS) {
        printf("inETHERNET_Cofig_Set d_ETHERNET_CONFIG_IP  failed\n");
        return (VS_ERROR);
    }
    /* Set Geteway IP */
    if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_GATEWAY, (unsigned char*) srEDCRec.szTermGetewayAddress, strlen(srEDCRec.szTermGetewayAddress)) != VS_SUCCESS) {
        printf("inETHERNET_Cofig_Set d_ETHERNET_CONFIG_GATEWAY  failed\n");
        return (VS_ERROR);
    }

    /* Set EDC SUB Mask */
    if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_MASK, (unsigned char*) srEDCRec.szTermMASKAddress, strlen(srEDCRec.szTermMASKAddress)) != VS_SUCCESS) {
        printf("inETHERNET_Cofig_Set d_ETHERNET_CONFIG_MASK  failed\n");
        return (VS_ERROR);
    }
    /*
            d_ETHERNET_CONFIG_UPDATE_EXIT
            Save the settings to the non-volatile memory in Ethernet module.      
     */
    /* 儲存設定 */
    memset(szConfig, 0x00, sizeof (szConfig));
    if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_UPDATE_EXIT, (unsigned char*) szConfig, 0) != VS_SUCCESS) {
        printf("inETHERNET_Cofig_Set  d_ETHERNET_CONFIG_UPDATE_EXIT  failed\n");
        return (VS_ERROR);
    }

    /* 魔法，勿動 START 
     * 這個Bug的發生時機在當機器直接斷電時，下次開機後，
     * 連線狀態會一直Timeout。
     * 正規解法是把設定IP、Submask、gateway 放到inETHERNET_SetConfig 再設定
     */
    {
//        memset(szGatWay, 0x00, sizeof (szGatWay));
//        inETHERNET_Cofig_Get(d_ETHERNET_CONFIG_GATEWAY, (unsigned char*) szGatWay, &uszLen);

        //		       if (inGetTermGetewayAddress(szGatWay) == VS_ERROR)
        //		       {
        //                            printf("inGetTermGetewayAddress   failed\n");
        //			       return (VS_ERROR);
        //		       }

        /* Set Geteway IP */
        if (inETHERNET_Cofig_Set(d_ETHERNET_CONFIG_GATEWAY, (unsigned char*) srEDCRec.szTermGetewayAddress, strlen(srEDCRec.szTermGetewayAddress)) != VS_SUCCESS) {
            printf("inETHERNET_Cofig_Set  d_ETHERNET_CONFIG_GATEWAY  again failed\n");
            return (VS_ERROR);
        }
    }
    /* 魔法，勿動 End */
    //	}
    return (VS_SUCCESS);
}
/* Describe:from example  */
void EthernetPing(const char* ip)
{
  BYTE PingBuff[15+1];
  sprintf(PingBuff,ip);
  unsigned short ret = CTOS_EthernetPing(PingBuff, strlen(PingBuff));
  if (ret == d_OK){
      printf("ping %s successed\n",PingBuff);
  }else{
      printf("ping %s failed,ret is %X\n",PingBuff,ret);
  }
}

int main(int argc, char *argv[]) {
    int i = 0;
    int inPrintIndex = 0, inRetVal;
    char szShort_Receipt_Mode[2 + 1];
    char szDebugMsg[100 + 1];
    char szCustomerIndicator[3 + 1] = {0};
    unsigned char uszBuffer[PB_CANVAS_X_SIZE * 8 * _BUFFER_MAX_LINE_];
    BufferHandle srBhandle;
    FONT_ATTRIB srFont_Attrib;
    TRANSACTION_OBJECT pobTran;
    //inFunc_ls("-R -l", _AP_ROOT_PATH_);
    inETHERNET_Initial();
    EthernetPing(hostIp);
    pobTran.srBRec.inPrintOption = _PRT_CUST_;
    strcpy(pobTran.srBRec.szCardLabel, "9中9文9字9"); //卡別
    strcpy(pobTran.srBRec.szPAN, "252500001616"); //卡號
    strcpy(pobTran.srBRec.szDate, "1223"); //日期
    strcpy(pobTran.srBRec.szTime, "143059"); //時間
    pobTran.srBRec.lnOrgInvNum = 2;
    pobTran.srBRec.lnBatchNum = 3;
    strcpy(pobTran.srBRec.szAuthCode, "123456");
    strcpy(pobTran.srBRec.szRefNo, "999999");

    inFunc_Booting_Flow_Print_Image_Initial(&pobTran);
    //    printf("初始化圖片預設高度\n");
    //    printf("%s:inBankLogoHeight is %d \n",     _BANK_LOGO_,      gsrBMPHeight.inBankLogoHeight);
    //    printf("%s:inMerchantLogoHeight is %d \n", _MERCHANT_LOGO_,  gsrBMPHeight.inMerchantLogoHeight);
    //    printf("%s:inTitleNameHeight is %d \n",    _NAME_LOGO_,      gsrBMPHeight.inTitleNameHeight);

    CTOS_PrinterFontSelectMode(d_FONT_TTF_MODE);

    //    CTOS_FontTTFSelectFontFile(d_FONT_DEVICE_PRINTER, "ca_default.ttf", 0);
    //    CTOS_PrinterPutString("Print API Test");
    //    CTOS_PrinterPutString("1測試1----");
    CTOS_FontTTFSelectFontFile(d_FONT_DEVICE_PRINTER, "tsuyuan.ttf", 0);
    CTOS_FontTTFSelectStyle(d_FONT_DEVICE_PRINTER, d_FONT_STYLE_NORMAL);

    inPRINT_Buffer_Initial(uszBuffer, _BUFFER_MAX_LINE_, &srFont_Attrib, &srBhandle);

    BYTE key;
    CTOS_LCDTClearDisplay();

    //    if ((inRetVal = inCREDIT_PRINT_Logo_ByBuffer(&pobTran, uszBuffer, &srFont_Attrib, &srBhandle)) != VS_SUCCESS)
    //        printf("inCREDIT_PRINT_Logo_ByBuffer failed, ret=%d\n",inRetVal);
    //    if ((inRetVal = inCREDIT_PRINT_Tidmid_ByBuffer(&pobTran, uszBuffer, &srFont_Attrib, &srBhandle)) != VS_SUCCESS)
    //        printf("inCREDIT_PRINT_Tidmid_ByBuffer failed, ret=%d\n",inRetVal);
    //    if ((inRetVal = inCREDIT_PRINT_Data_ByBuffer(&pobTran, uszBuffer, &srFont_Attrib, &srBhandle)) != VS_SUCCESS)
    //        printf("inCREDIT_PRINT_Data_ByBuffer failed, ret=%d\n",inRetVal);
    //    if ((inRetVal = inPRINT_Buffer_OutPut(uszBuffer, &srBhandle)) != VS_SUCCESS)
    //        printf("inPRINT_Buffer_OutPut failed, ret=%d\n",inRetVal);

    CTOS_LCDTPrintXY(1, 1, "show on LCD 123"); //console  to the  LCD
    CTOS_KBDGet(&key);
    exit(0);
}
