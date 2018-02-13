//this file is part of notepad++
//Copyright (C)2003 Don HO <donho@altern.org>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "PluginDefinition.h"
#include <shlwapi.h>
#include "GoToLineDlg.h"
#include <windows.h>
#include <iostream>
#include "../Dragbase/dragbase/dragbase/dragbase.h"
extern FuncItem funcItem[nbFunc];
extern NppData nppData;
extern bool doCloseTag;

extern DemoDlg _goToLine;

INT_PTR CALLBACK InputDialogProc(
	__in  HWND hwndDlg,
	__in  UINT uMsg,
	__in  WPARAM wParam,
	__in  LPARAM lParam
);

// Param struct passed to the callback procs
struct CallbackParam
{
	HWND hwnd;
	HINSTANCE hinst;
	bool * new_document;
};

const unsigned MAX_TEXT_SIZE = 1000000;
// *** Our save procecure which is also used by dragbase
BOOL __stdcall Save(const char * const filename, const char * const directory, void * param)
{
	CallbackParam * cp = (CallbackParam*)param;

	// Pointer to the real file name (including path)
	const char * filename_real = filename;

	// Only show the dialog to enter a file name if there is a new document
	// (with no document name yet)
	if (*cp->new_document)
	{
		char filename_tmp[4096];
			SetFilename(filename_tmp);
			*cp->new_document = false;
			// Create new path with file name (because filename_tmp is only the file name!)
			char new_filename[4096];
			strcpy(new_filename, directory);
			strcat(new_filename, filename_tmp);
			filename_real = filename_tmp;
	}


	char *txt_content = new char[MAX_TEXT_SIZE + 1];
	GetWindowText(cp->hwnd, txt_content, MAX_TEXT_SIZE);



	delete[] txt_content;

	return true;
}

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  reasonForCall, 
                       LPVOID lpReserved )
{
	HWND hNotepad_plus = FindWindow(TEXT("Notepad++"), NULL);
	Create(hNotepad_plus, Save,"Testing",NULL);
    switch (reasonForCall)
    {

      case DLL_PROCESS_ATTACH:
        pluginInit(hModule);
        break;

      case DLL_PROCESS_DETACH:
		commandMenuCleanUp();
        pluginCleanUp();
        break;

      case DLL_THREAD_ATTACH:
        break;

      case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}


extern "C" __declspec(dllexport) void setInfo(NppData notpadPlusData)
{
	nppData = notpadPlusData;
	commandMenuInit();
}

extern "C" __declspec(dllexport) const TCHAR * getName()
{
	return NPP_PLUGIN_NAME;
}

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbF)
{
	*nbF = nbFunc;
	return funcItem;
}


extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode)
{
	switch (notifyCode->nmhdr.code) 
	{
		case SCN_CHARADDED:
		{
			LangType docType;
			::SendMessage(nppData._nppHandle, NPPM_GETCURRENTLANGTYPE, 0, (LPARAM)&docType);
			bool isDocTypeHTML = (docType == L_HTML || docType == L_XML || docType == L_PHP);
			if (doCloseTag && isDocTypeHTML)
			{
				if (notifyCode->ch == '>')
				{
					char buf[512];
					int currentEdit;
					::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
					HWND hCurrentEditView = (currentEdit == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;
					int currentPos = int(::SendMessage(hCurrentEditView, SCI_GETCURRENTPOS, 0, 0));
					int beginPos = currentPos - (sizeof(buf) - 1);
					int startPos = (beginPos > 0)?beginPos:0;
					int size = currentPos - startPos;
					int insertStringSize = 2;
					char insertString[516] = "</";

					if (size >= 3) 
					{
						struct TextRange tr = {{startPos, currentPos}, buf};

						::SendMessage(hCurrentEditView, SCI_GETTEXTRANGE, 0, (LPARAM)&tr);

						if (buf[size-2] != '/') 
						{

							const char *pBegin = &buf[0];
							const char *pCur = &buf[size - 2];
							int  insertStringSize = 2;

							for (; pCur > pBegin && *pCur != '<' && *pCur != '>' ;)
								pCur--;
								

							if (*pCur == '<')
							{
								pCur++;
								
								while (StrChrA(":_-.", *pCur) || IsCharAlphaNumeric(*pCur))
								{
									insertString[insertStringSize++] = *pCur;
									pCur++;
								}
							}

							insertString[insertStringSize++] = '>';
							insertString[insertStringSize] = '\0';

							if (insertStringSize > 3)
							{				
								::SendMessage(hCurrentEditView, SCI_BEGINUNDOACTION, 0, 0);
								::SendMessage(hCurrentEditView, SCI_REPLACESEL, 0, (LPARAM)insertString);
								::SendMessage(hCurrentEditView, SCI_SETSEL, currentPos, currentPos);
								::SendMessage(hCurrentEditView, SCI_ENDUNDOACTION, 0, 0);
							}
						}
					}	
				}
			}
		}
		break;
	}
}


// Here you can process the Npp Messages 
// I will make the messages accessible little by little, according to the need of plugin development.
// Please let me know if you need to access to some messages :
// http://sourceforge.net/forum/forum.php?forum_id=482781
//
extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam)
{/*
	if (Message == WM_MOVE)
	{
		::MessageBox(NULL, "move", "", MB_OK);
	}
*/
	return TRUE;
}

#ifdef UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode()
{
    return TRUE;
}
#endif //UNICODE
