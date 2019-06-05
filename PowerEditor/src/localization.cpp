// This file is part of Notepad++ project
// Copyright (C)2003 Don HO <don.h@free.fr>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// Note that the GPL places important restrictions on "derived works", yet
// it does not provide a detailed definition of that term.  To avoid      
// misunderstandings, we consider an application to constitute a          
// "derivative work" for the purpose of this license if it does any of the
// following:                                                             
// 1. Integrates source code from Notepad++.
// 2. Integrates/includes/aggregates Notepad++ into a proprietary executable
//    installer, such as those produced by InstallShield.
// 3. Links to a library or executes a program that does any of the above.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


#include "Notepad_plus.h"
#include "ShortcutMapper.h"
#include "EncodingMapper.h"
#include "localization.h"

using namespace std;



MenuPosition menuPos[] = {
	//==============================================
	//  {L0,  L1,  L2,    id},
	//==============================================
	{ 0, -1, -1, "file" },
	{ 1, -1, -1, "edit" },
	{ 2, -1, -1, "search" },
	{ 3, -1, -1, "view" },
	{ 4, -1, -1, "encoding" },
	{ 5, -1, -1, "language" },
	{ 6, -1, -1, "settings" },
	{ 7, -1, -1, "tools" },
	{ 8, -1, -1, "macro" },
	{ 9, -1, -1, "run" },

	{ 0,  2, -1, "file-openFolder" },
	{ 0, 13, -1, "file-closeMore" },
	{ 0, 22, -1, "file-recentFiles" },

	{ 1, 10, -1, "edit-copyToClipboard" },
	{ 1, 11, -1, "edit-indent" },
	{ 1, 12, -1, "edit-convertCaseTo" },
	{ 1, 13, -1, "edit-lineOperations" },
	{ 1, 14, -1, "edit-comment" },
	{ 1, 15, -1, "edit-autoCompletion" },
	{ 1, 16, -1, "edit-eolConversion" },
	{ 1, 17, -1, "edit-blankOperations" },
	{ 1, 18, -1, "edit-pasteSpecial" },
	{ 1, 19, -1, "edit-onSelection" },

	{ 2, 18, -1, "search-markAll" },
	{ 2, 19, -1, "search-unmarkAll" },
	{ 2, 20, -1, "search-jumpUp" },
	{ 2, 21, -1, "search-jumpDown" },
	{ 2, 23, -1, "search-bookmark" },

	{ 3,  4, -1, "view-currentFileIn" },
	{ 3,  6, -1, "view-showSymbol" },
	{ 3,  7, -1, "view-zoom" },
	{ 3,  8, -1, "view-moveCloneDocument" },
	{ 3,  9, -1, "view-tab" },
	{ 3, 18, -1, "view-collapseLevel" },
	{ 3, 19, -1, "view-uncollapseLevel" },
	{ 3, 23, -1, "view-project" },

	{ 4,  5, -1, "encoding-characterSets" },
	{ 4,  5,  0, "encoding-arabic" },
	{ 4,  5,  1, "encoding-baltic" },
	{ 4,  5,  2, "encoding-celtic" },
	{ 4,  5,  3, "encoding-cyrillic" },
	{ 4,  5,  4, "encoding-centralEuropean" },
	{ 4,  5,  5, "encoding-chinese" },
	{ 4,  5,  6, "encoding-easternEuropean" },
	{ 4,  5,  7, "encoding-greek" },
	{ 4,  5,  8, "encoding-hebrew" },
	{ 4,  5,  9, "encoding-japanese" },
	{ 4,  5, 10, "encoding-korean" },
	{ 4,  5, 11, "encoding-northEuropean" },
	{ 4,  5, 12, "encoding-thai" },
	{ 4,  5, 13, "encoding-turkish" },
	{ 4,  5, 14, "encoding-westernEuropean" },
	{ 4,  5, 15, "encoding-vietnamese" },

	{ 6,  4, -1, "settings-import" },

	{ 7,  0, -1, "tools-md5" },
	{ 7,  1, -1, "tools-sha256" },
	{ -1, -1, -1, "" } // End of array
};

void NativeLangSpeaker::init(TiXmlDocumentA *nativeLangDocRootA, bool loadIfEnglish)
{
	if (nativeLangDocRootA)
	{
		_nativeLangA =  nativeLangDocRootA->FirstChild("NotepadPlus");
		if (_nativeLangA)
		{
			_nativeLangA = _nativeLangA->FirstChild("Native-Langue");
			if (_nativeLangA)
			{
				TiXmlElementA *element = _nativeLangA->ToElement();
				const char *rtl = element->Attribute("RTL");
				if (rtl)
					_isRTL = (strcmp(rtl, "yes") == 0);
                else
                    _isRTL = false;

                // get original file name (defined by Notpad++) from the attribute
                _fileName = element->Attribute("filename");

				if (!loadIfEnglish && _fileName && stricmp("english.xml", _fileName) == 0)
                {
					_nativeLangA = NULL;
					return;
				}
				// get encoding
				TiXmlDeclarationA *declaration =  _nativeLangA->GetDocument()->FirstChild()->ToDeclaration();
				if (declaration)
				{
					const char * encodingStr = declaration->Encoding();
					EncodingMapper *em = EncodingMapper::getInstance();
                    int enc = em->getEncodingFromString(encodingStr);
                    _nativeLangEncoding = (enc != -1)?enc:CP_ACP;
				}
			}	
		}
    }
}

generic_string NativeLangSpeaker::getSpecialMenuEntryName(const char *entryName) const
{
	if (!_nativeLangA) return TEXT("");
	TiXmlNodeA *mainMenu = _nativeLangA->FirstChild("Menu");
	if (!mainMenu) return TEXT("");
	mainMenu = mainMenu->FirstChild("Main");
	if (!mainMenu) return TEXT("");
	TiXmlNodeA *entriesRoot = mainMenu->FirstChild("Entries");
	if (!entriesRoot) return TEXT("");

	WcharMbcsConvertor *wmc = WcharMbcsConvertor::getInstance();

	for (TiXmlNodeA *childNode = entriesRoot->FirstChildElement("Item");
		childNode ;
		childNode = childNode->NextSibling("Item") )
	{
		TiXmlElementA *element = childNode->ToElement();

		const char *idName = element->Attribute("idName");
		if (idName)
		{
			const char *name = element->Attribute("name");
			if (!strcmp(idName, entryName))
			{
				return wmc->char2wchar(name, _nativeLangEncoding);
			}
		}
	}
	return TEXT("");
}

generic_string NativeLangSpeaker::getNativeLangMenuString(int itemID) const
{
	if (!_nativeLangA)
		return TEXT("");

	TiXmlNodeA *node = _nativeLangA->FirstChild("Menu");
	if (!node) return TEXT("");

	node = node->FirstChild("Main");
	if (!node) return TEXT("");

	node = node->FirstChild("Commands");
	if (!node) return TEXT("");

	WcharMbcsConvertor *wmc = WcharMbcsConvertor::getInstance();

	for (TiXmlNodeA *childNode = node->FirstChildElement("Item");
		childNode ;
		childNode = childNode->NextSibling("Item") )
	{
		TiXmlElementA *element = childNode->ToElement();
		int id;
		if (element->Attribute("id", &id) && (id == itemID))
		{
			const char *name = element->Attribute("name");
			if (name)
			{
				return wmc->char2wchar(name, _nativeLangEncoding);
			}
		}
	}
	return TEXT("");
}

generic_string NativeLangSpeaker::getLocalizedStrFromID(const char *strID, const generic_string& defaultString) const
{
	if (not _nativeLangA)
		return defaultString;

	if (not strID)
		return defaultString;

	TiXmlNodeA *node = _nativeLangA->FirstChild("MiscStrings");
	if (not node) return defaultString;

	node = node->FirstChild(strID);
	if (not node) return defaultString;

	TiXmlElementA *element = node->ToElement();

	const char *value = element->Attribute("value");
	if (not value) return defaultString;

	WcharMbcsConvertor *wmc = WcharMbcsConvertor::getInstance();
	return wmc->char2wchar(value, _nativeLangEncoding);
}



MenuPosition & getMenuPosition(const char *id)
{

	int nbSubMenuPos = sizeof(menuPos)/sizeof(MenuPosition);

	for (int i = 0; i < nbSubMenuPos; ++i) 
	{
		if (strcmp(menuPos[i]._id, id) == 0)
			return menuPos[i];
	}
	return menuPos[nbSubMenuPos-1];
}

void NativeLangSpeaker::changeMenuLang(HMENU menuHandle, generic_string & pluginsTrans, generic_string & windowTrans)
{
	if (nullptr == _nativeLangA)
		return;

	TiXmlNodeA *mainMenu = _nativeLangA->FirstChild("Menu");
	if (nullptr == mainMenu)
		return;

	mainMenu = mainMenu->FirstChild("Main");
	if (nullptr == mainMenu)
		return;

	TiXmlNodeA *entriesRoot = mainMenu->FirstChild("Entries");
	if (nullptr == entriesRoot)
		return;

	const char* idName = nullptr;
	WcharMbcsConvertor* wmc = WcharMbcsConvertor::getInstance();

	for (TiXmlNodeA *childNode = entriesRoot->FirstChildElement("Item");
		childNode ;
		childNode = childNode->NextSibling("Item") )
	{
		TiXmlElementA *element = childNode->ToElement();
		const char *menuIdStr = element->Attribute("menuId");
		if (menuIdStr)
		{
			MenuPosition & menuPos = getMenuPosition(menuIdStr);
			if (menuPos._x != -1)
			{
				const char *name = element->Attribute("name");
				const wchar_t *nameW = wmc->char2wchar(name, _nativeLangEncoding);
				::ModifyMenu(menuHandle, menuPos._x, MF_BYPOSITION, 0, nameW);
			}
		}
		else
		{
			idName = element->Attribute("idName");
			if (idName)
			{
				const char *name = element->Attribute("name");
				if (!strcmp(idName, "Plugins"))
				{
					const wchar_t *nameW = wmc->char2wchar(name, _nativeLangEncoding);
					pluginsTrans = nameW;
				}
				else if (!strcmp(idName, "Window"))
				{
					const wchar_t *nameW = wmc->char2wchar(name, _nativeLangEncoding);
					windowTrans = nameW;
				}
			}
		}
	}

	TiXmlNodeA *menuCommandsRoot = mainMenu->FirstChild("Commands");
	for (TiXmlNodeA *childNode = menuCommandsRoot->FirstChildElement("Item");
		childNode ;
		childNode = childNode->NextSibling("Item") )
	{
		TiXmlElementA *element = childNode->ToElement();
		int id;
		element->Attribute("id", &id);
		const char *name = element->Attribute("name");

		const wchar_t *nameW = wmc->char2wchar(name, _nativeLangEncoding);
		::ModifyMenu(menuHandle, id, MF_BYCOMMAND, id, nameW);
	}

	TiXmlNodeA *subEntriesRoot = mainMenu->FirstChild("SubEntries");

	for (TiXmlNodeA *childNode = subEntriesRoot->FirstChildElement("Item");
		childNode ;
		childNode = childNode->NextSibling("Item") )
	{
		TiXmlElementA* element = childNode->ToElement();
		//const char *xStr = element->Attribute("posX", &x);
		//const char *yStr = element->Attribute("posY", &y);
		const char* subMenuIdStr = element->Attribute("subMenuId");
		const char* name = element->Attribute("name");

		if (nullptr == subMenuIdStr or nullptr == name)
			continue;

		MenuPosition& menuPos = getMenuPosition(subMenuIdStr);
		int x = menuPos._x;
		int y = menuPos._y;
		int z = menuPos._z;

		HMENU hSubMenu = ::GetSubMenu(menuHandle, x);
		if (!hSubMenu)
			continue;

		HMENU hSubMenu2 = ::GetSubMenu(hSubMenu, y);
		if (!hSubMenu2)
			continue;

		HMENU hMenu = hSubMenu;
		int pos = y;

		//const char *zStr = element->Attribute("posZ", &z);
		if (z != -1)
		{
			HMENU hSubMenu3 = ::GetSubMenu(hSubMenu2, z);
			if (!hSubMenu3)
				continue;
			hMenu = hSubMenu2;
			pos = z;
		}

		const wchar_t *nameW = wmc->char2wchar(name, _nativeLangEncoding);
		::ModifyMenu(hMenu, pos, MF_BYPOSITION, 0, nameW);
	}
}


static const int tabContextMenuItemPos[] =
{
//  +-------------- The order in tab menu (NppNotification.cpp : if (!_tabPopupMenu.isCreated())
//  |
//  |       +------ Number in english.xml (<language>.xml) : <TabBar>
//  |       |
    0,   // 0 : Close
    1,   // 1 : Close ALL BUT This
    5,   // 2 : Save
    6,   // 3 : Save As
    10,  // 4 : Print
    24,  // 5 : Move to Other View
    25,  // 6 : Clone to Other View
    20,  // 7 : Full File Path to Clipboard
    21,  // 8 : Filename to Clipboard
    22,  // 9 : Current Dir. Path to Clipboard
    7,   // 10: Rename
    8,   // 11: Move to Recycle Bin
    17,  // 12: Read-Only
    18,  // 13: Clear Read-Only Flag
    26,  // 14: Move to New Instance
    27,  // 15: Open to New Instance
    9,   // 16: Reload
    2,   // 17: Close ALL to the Left
    3,   // 18: Close ALL to the Right
    12,  // 19: Open Containing Folder in Explorer
    13,  // 20: Open Containing Folder in cmd
    15,  // 21: Open in Default Viewer
    4,   // 22: Close ALL Unchanged
    -1   //-------End
};


void NativeLangSpeaker::changeLangTabContextMenu(HMENU hCM)
{
	if (nullptr != _nativeLangA)
	{
		TiXmlNodeA *tabBarMenu = _nativeLangA->FirstChild("Menu");
		if (tabBarMenu)
		{
			tabBarMenu = tabBarMenu->FirstChild("TabBar");
			if (tabBarMenu)
			{
				WcharMbcsConvertor* wmc = WcharMbcsConvertor::getInstance();
				int nbCMItems = sizeof(tabContextMenuItemPos)/sizeof(int);

				for (TiXmlNodeA *childNode = tabBarMenu->FirstChildElement("Item");
					childNode ;
					childNode = childNode->NextSibling("Item") )
				{
					TiXmlElementA *element = childNode->ToElement();
					int index;
					const char *indexStr = element->Attribute("CMID", &index);
					if (!indexStr || (index < 0 || index >= nbCMItems-1))
						continue;

					int pos = tabContextMenuItemPos[index];
					const char *pName = element->Attribute("name");
					if (pName)
					{
						const wchar_t *pNameW = wmc->char2wchar(pName, _nativeLangEncoding);
						int cmdID = ::GetMenuItemID(hCM, pos);
						::ModifyMenu(hCM, pos, MF_BYPOSITION, cmdID, pNameW);
					}
				}
			}
		}
	}
}

void NativeLangSpeaker::changeLangTabDrapContextMenu(HMENU hCM)
{
	const int POS_GO2VIEW = 0;
	const int POS_CLONE2VIEW = 1;

	if (_nativeLangA)
	{
		const char *goToViewA = nullptr;
		const char *cloneToViewA = nullptr;

		TiXmlNodeA *tabBarMenu = _nativeLangA->FirstChild("Menu");
		if (tabBarMenu)
			tabBarMenu = tabBarMenu->FirstChild("TabBar");

		if (tabBarMenu)
		{
			for (TiXmlNodeA *childNode = tabBarMenu->FirstChildElement("Item");
				childNode ;
				childNode = childNode->NextSibling("Item") )
			{
				TiXmlElementA *element = childNode->ToElement();
				int ordre;
				element->Attribute("CMID", &ordre);
				if (ordre == 5)
					goToViewA = element->Attribute("name");
				else if (ordre == 6)
					cloneToViewA = element->Attribute("name");
			}
		}

		WcharMbcsConvertor *wmc = WcharMbcsConvertor::getInstance();
		if (goToViewA && goToViewA[0])
		{
			const wchar_t *goToViewG = wmc->char2wchar(goToViewA, _nativeLangEncoding);
			int cmdID = ::GetMenuItemID(hCM, POS_GO2VIEW);
			::ModifyMenu(hCM, POS_GO2VIEW, MF_BYPOSITION|MF_STRING, cmdID, goToViewG);
		}
		if (cloneToViewA && cloneToViewA[0])
		{
			const wchar_t *cloneToViewG = wmc->char2wchar(cloneToViewA, _nativeLangEncoding);
			int cmdID = ::GetMenuItemID(hCM, POS_CLONE2VIEW);
			::ModifyMenu(hCM, POS_CLONE2VIEW, MF_BYPOSITION|MF_STRING, cmdID, cloneToViewG);
		}
	}
}


void NativeLangSpeaker::changeConfigLang(HWND hDlg)
{
	if (nullptr == _nativeLangA)
		return;

	TiXmlNodeA *styleConfDlgNode = _nativeLangA->FirstChild("Dialog");
	if (!styleConfDlgNode)
		return;

	styleConfDlgNode = styleConfDlgNode->FirstChild("StyleConfig");
	if (!styleConfDlgNode) return;

	WcharMbcsConvertor *wmc = WcharMbcsConvertor::getInstance();

	// Set Title
	const char *titre = (styleConfDlgNode->ToElement())->Attribute("title");

	if ((titre && titre[0]) && hDlg)
	{
		const wchar_t *nameW = wmc->char2wchar(titre, _nativeLangEncoding);
		::SetWindowText(hDlg, nameW);
	}
	for (TiXmlNodeA *childNode = styleConfDlgNode->FirstChildElement("Item");
		childNode ;
		childNode = childNode->NextSibling("Item") )
	{
		TiXmlElementA *element = childNode->ToElement();
		int id;
		const char *sentinel = element->Attribute("id", &id);
		const char *name = element->Attribute("name");
		if (sentinel && (name && name[0]))
		{
			HWND hItem = ::GetDlgItem(hDlg, id);
			if (hItem)
			{
				const wchar_t *nameW = wmc->char2wchar(name, _nativeLangEncoding);
				::SetWindowText(hItem, nameW);
			}
		}
	}
	styleConfDlgNode = styleConfDlgNode->FirstChild("SubDialog");
	
	for (TiXmlNodeA *childNode = styleConfDlgNode->FirstChildElement("Item");
		childNode ;
		childNode = childNode->NextSibling("Item") )
	{
		TiXmlElementA *element = childNode->ToElement();
		int id;
		const char *sentinel = element->Attribute("id", &id);
		const char *name = element->Attribute("name");
		if (sentinel && (name && name[0]))
		{
			HWND hItem = ::GetDlgItem(hDlg, id);
			if (hItem)
			{
				const wchar_t *nameW = wmc->char2wchar(name, _nativeLangEncoding);
				::SetWindowText(hItem, nameW);
			}
		}
	}
}


void NativeLangSpeaker::changeStyleCtrlsLang(HWND hDlg, int *idArray, const char **translatedText)
{
	const int iColorStyle = 0;
	const int iUnderline = 8;

	HWND hItem;
	for (int i = iColorStyle ; i < (iUnderline + 1) ; ++i)
	{
		if (translatedText[i] && translatedText[i][0])
		{
			hItem = ::GetDlgItem(hDlg, idArray[i]);
			if (hItem)
			{
				WcharMbcsConvertor *wmc = WcharMbcsConvertor::getInstance();
				const wchar_t *nameW = wmc->char2wchar(translatedText[i], _nativeLangEncoding);
				::SetWindowText(hItem, nameW);
			}
		}
	}
}

void NativeLangSpeaker::changeUserDefineLangPopupDlg(HWND hDlg)
{
	if (!_nativeLangA) return;

	TiXmlNodeA *userDefineDlgNode = _nativeLangA->FirstChild("Dialog");
	if (!userDefineDlgNode) return;	
	
	userDefineDlgNode = userDefineDlgNode->FirstChild("UserDefine");
	if (!userDefineDlgNode) return;

	WcharMbcsConvertor *wmc = WcharMbcsConvertor::getInstance();

	TiXmlNodeA *stylerDialogNode = userDefineDlgNode->FirstChild("StylerDialog");
	if (!stylerDialogNode) return;

	const char *titre = (stylerDialogNode->ToElement())->Attribute("title");
	if (titre &&titre[0])
	{
		const wchar_t *nameW = wmc->char2wchar(titre, _nativeLangEncoding);
		::SetWindowText(hDlg, nameW);
	}
	for (TiXmlNodeA *childNode = stylerDialogNode->FirstChildElement("Item");
		childNode ;
		childNode = childNode->NextSibling("Item") )
	{
		TiXmlElementA *element = childNode->ToElement();
		int id;
		const char *sentinel = element->Attribute("id", &id);
		const char *name = element->Attribute("name");
		if (sentinel && (name && name[0]))
		{
			HWND hItem = ::GetDlgItem(hDlg, id);
			if (hItem)
			{
				const wchar_t *nameW = wmc->char2wchar(name, _nativeLangEncoding);
				::SetWindowText(hItem, nameW);

			}
		}
	}
}

void NativeLangSpeaker::changeUserDefineLang(UserDefineDialog *userDefineDlg)
{
	if (!_nativeLangA) return;

	TiXmlNodeA *userDefineDlgNode = _nativeLangA->FirstChild("Dialog");
	if (!userDefineDlgNode) return;	
	
	userDefineDlgNode = userDefineDlgNode->FirstChild("UserDefine");
	if (!userDefineDlgNode) return;

	HWND hDlg = userDefineDlg->getHSelf();

	WcharMbcsConvertor *wmc = WcharMbcsConvertor::getInstance();

	// Set Title
	const char *titre = (userDefineDlgNode->ToElement())->Attribute("title");
	if (titre && titre[0])
	{
		const wchar_t *nameW = wmc->char2wchar(titre, _nativeLangEncoding);
		::SetWindowText(hDlg, nameW);
	}
	// for each control
	const int nbControl = 9;
	const char *translatedText[nbControl];
	for (int i = 0 ; i < nbControl ; ++i)
		translatedText[i] = NULL;

	for (TiXmlNodeA *childNode = userDefineDlgNode->FirstChildElement("Item");
		childNode ;
		childNode = childNode->NextSibling("Item") )
	{
		TiXmlElementA *element = childNode->ToElement();
		int id;
		const char *sentinel = element->Attribute("id", &id);
		const char *name = element->Attribute("name");
		
		if (sentinel && (name && name[0]))
		{
			if (id > 30)
			{
				HWND hItem = ::GetDlgItem(hDlg, id);
				if (hItem)
				{
					const wchar_t *nameW = wmc->char2wchar(name, _nativeLangEncoding);
					::SetWindowText(hItem, nameW);
				}
			}
			else
			{
				switch(id)
				{
					case 0: case 1: case 2: case 3: case 4:
					case 5: case 6: case 7: case 8: 
 						translatedText[id] = name; break;
				}
			}
		}
	}
	const int nbDlg = 4;
	HWND hDlgArrary[nbDlg];
	hDlgArrary[0] = userDefineDlg->getFolderHandle();
	hDlgArrary[1] = userDefineDlg->getKeywordsHandle();
	hDlgArrary[2] = userDefineDlg->getCommentHandle();
	hDlgArrary[3] = userDefineDlg->getSymbolHandle();

	const char nodeNameArray[nbDlg][16] = {"Folder", "Keywords", "Comment", "Operator"};

	for (int i = 0 ; i < nbDlg ; ++i)
	{
		TiXmlNodeA *node = userDefineDlgNode->FirstChild(nodeNameArray[i]);
		
		if (node) 
		{
			// Set Title
			titre = (node->ToElement())->Attribute("title");
			if (titre &&titre[0])
			{
				const wchar_t *nameW = wmc->char2wchar(titre, _nativeLangEncoding);
				userDefineDlg->setTabName(i, nameW);
			}
			for (TiXmlNodeA *childNode = node->FirstChildElement("Item");
				childNode ;
				childNode = childNode->NextSibling("Item") )
			{
				TiXmlElementA *element = childNode->ToElement();
				int id;
				const char *sentinel = element->Attribute("id", &id);
				const char *name = element->Attribute("name");
				if (sentinel && (name && name[0]))
				{
					HWND hItem = ::GetDlgItem(hDlgArrary[i], id);
					if (hItem)
					{
						const wchar_t *nameW = wmc->char2wchar(name, _nativeLangEncoding);
						::SetWindowText(hItem, nameW);
					}
				}
			}
		}
	}
}

void NativeLangSpeaker::changeFindReplaceDlgLang(FindReplaceDlg & findReplaceDlg)
{
	if (_nativeLangA)
	{
		TiXmlNodeA *dlgNode = _nativeLangA->FirstChild("Dialog");
		if (dlgNode)
		{
			NppParameters *pNppParam = NppParameters::getInstance();
			dlgNode = searchDlgNode(dlgNode, "Find");
			if (dlgNode)
			{
				const char *titre1 = (dlgNode->ToElement())->Attribute("titleFind");
				const char *titre2 = (dlgNode->ToElement())->Attribute("titleReplace");
				const char *titre3 = (dlgNode->ToElement())->Attribute("titleFindInFiles");
				const char *titre4 = (dlgNode->ToElement())->Attribute("titleMark");

				WcharMbcsConvertor *wmc = WcharMbcsConvertor::getInstance();

				if (titre1 && titre1[0])
				{
					basic_string<wchar_t> nameW = wmc->char2wchar(titre1, _nativeLangEncoding);
					pNppParam->getFindDlgTabTitiles()._find = nameW;
					findReplaceDlg.changeTabName(FIND_DLG, pNppParam->getFindDlgTabTitiles()._find.c_str());
				}
				if (titre2  && titre2[0])
				{
					basic_string<wchar_t> nameW = wmc->char2wchar(titre2, _nativeLangEncoding);
					pNppParam->getFindDlgTabTitiles()._replace = nameW;
					findReplaceDlg.changeTabName(REPLACE_DLG, pNppParam->getFindDlgTabTitiles()._replace.c_str());
				}
				if (titre3 && titre3[0])
				{
					basic_string<wchar_t> nameW = wmc->char2wchar(titre3, _nativeLangEncoding);
					pNppParam->getFindDlgTabTitiles()._findInFiles = nameW;
					findReplaceDlg.changeTabName(FINDINFILES_DLG, pNppParam->getFindDlgTabTitiles()._findInFiles.c_str());
				}
				if (titre4 && titre4[0])
				{
					basic_string<wchar_t> nameW = wmc->char2wchar(titre4, _nativeLangEncoding);
					pNppParam->getFindDlgTabTitiles()._mark = nameW;
					findReplaceDlg.changeTabName(MARK_DLG, pNppParam->getFindDlgTabTitiles()._mark.c_str());
				}
			}
		}
	}
	changeDlgLang(findReplaceDlg.getHSelf(), "Find");
}

void NativeLangSpeaker::changePluginsAdminDlgLang(PluginsAdminDlg & pluginsAdminDlg)
{
	if (_nativeLangA)
	{
		TiXmlNodeA *dlgNode = _nativeLangA->FirstChild("Dialog");
		if (dlgNode)
		{
			dlgNode = searchDlgNode(dlgNode, "PluginsAdminDlg");
			if (dlgNode)
			{
				WcharMbcsConvertor *wmc = WcharMbcsConvertor::getInstance();

				TiXmlNodeA *ColumnPluginNode = dlgNode->FirstChild("ColumnPlugin");
				if (ColumnPluginNode)
				{
					const char *name = (ColumnPluginNode->ToElement())->Attribute("name");
					if (name && name[0])
					{
						basic_string<wchar_t> nameW = wmc->char2wchar(name, _nativeLangEncoding);
						pluginsAdminDlg.changeColumnName(COLUMN_PLUGIN, nameW.c_str());
					}
				}

				TiXmlNodeA *ColumnVersionNode = dlgNode->FirstChild("ColumnVersion");
				if (ColumnVersionNode)
				{
					const char *name = (ColumnVersionNode->ToElement())->Attribute("name");
					if (name && name[0])
					{
						basic_string<wchar_t> nameW = wmc->char2wchar(name, _nativeLangEncoding);
						pluginsAdminDlg.changeColumnName(COLUMN_VERSION, nameW.c_str());
					}
				}

				TiXmlNodeA *ColumnDateNode = dlgNode->FirstChild("ColumnDate");
				if (ColumnDateNode)
				{
					const char *name = (ColumnDateNode->ToElement())->Attribute("name");
					if (name && name[0])
					{
						basic_string<wchar_t> nameW = wmc->char2wchar(name, _nativeLangEncoding);
						pluginsAdminDlg.changeColumnName(COLUMN_DATE, nameW.c_str());
					}
				}

				const char *titre1 = (dlgNode->ToElement())->Attribute("titleAvailable");
				const char *titre2 = (dlgNode->ToElement())->Attribute("titleUpdates");
				const char *titre3 = (dlgNode->ToElement())->Attribute("titleInstalled");

				if (titre1 && titre1[0])
				{
					basic_string<wchar_t> nameW = wmc->char2wchar(titre1, _nativeLangEncoding);
					pluginsAdminDlg.changeTabName(AVAILABLE_LIST, nameW.c_str());
				}
				if (titre2  && titre2[0])
				{
					basic_string<wchar_t> nameW = wmc->char2wchar(titre2, _nativeLangEncoding);
					pluginsAdminDlg.changeTabName(UPDATES_LIST, nameW.c_str());
				}
				if (titre3 && titre3[0])
				{
					basic_string<wchar_t> nameW = wmc->char2wchar(titre3, _nativeLangEncoding);
					pluginsAdminDlg.changeTabName(INSTALLED_LIST, nameW.c_str());
				}
			}

			changeDlgLang(pluginsAdminDlg.getHSelf(), "PluginsAdminDlg");
		}
	}
}

void NativeLangSpeaker::changePrefereceDlgLang(PreferenceDlg & preference) 
{
	auto currentSel = preference.getListSelectedIndex();
	changeDlgLang(preference.getHSelf(), "Preference");

	WcharMbcsConvertor *wmc = WcharMbcsConvertor::getInstance();
	const size_t titreMaxSize = 128;
	char titre[titreMaxSize];
	changeDlgLang(preference._barsDlg.getHSelf(), "Global", titre, titreMaxSize);
	if (titre[0] != '\0')
	{
		const wchar_t *nameW = wmc->char2wchar(titre, _nativeLangEncoding);
		preference.renameDialogTitle(TEXT("Global"), nameW);
	}
	changeDlgLang(preference._marginsDlg.getHSelf(), "Scintillas", titre, titreMaxSize);
	if (titre[0] != '\0')
	{
		const wchar_t *nameW = wmc->char2wchar(titre, _nativeLangEncoding);
		preference.renameDialogTitle(TEXT("Scintillas"), nameW);
	}

	changeDlgLang(preference._defaultNewDocDlg.getHSelf(), "NewDoc", titre, titreMaxSize);
	if (titre[0] != '\0')
	{
		const wchar_t *nameW = wmc->char2wchar(titre, _nativeLangEncoding);
		preference.renameDialogTitle(TEXT("NewDoc"), nameW);
	}

	changeDlgLang(preference._defaultDirectoryDlg.getHSelf(), "DefaultDir", titre, titreMaxSize);
	if (titre[0] != '\0')
	{
		const wchar_t *nameW = wmc->char2wchar(titre, _nativeLangEncoding);
		preference.renameDialogTitle(TEXT("DefaultDir"), nameW);
	}

	changeDlgLang(preference._recentFilesHistoryDlg.getHSelf(), "RecentFilesHistory", titre, titreMaxSize);
	if (titre[0] != '\0')
	{
		const wchar_t *nameW = wmc->char2wchar(titre, _nativeLangEncoding);
		preference.renameDialogTitle(TEXT("RecentFilesHistory"), nameW);
	}

	changeDlgLang(preference._fileAssocDlg.getHSelf(), "FileAssoc", titre, titreMaxSize);
	if (titre[0] != '\0')
	{
		const wchar_t *nameW = wmc->char2wchar(titre, _nativeLangEncoding);
		preference.renameDialogTitle(TEXT("FileAssoc"), nameW);
	}

	changeDlgLang(preference._langMenuDlg.getHSelf(), "Language", titre, titreMaxSize);
	if (titre[0] != '\0')
	{
		const wchar_t *nameW = wmc->char2wchar(titre, _nativeLangEncoding);
		preference.renameDialogTitle(TEXT("Language"), nameW);
	}

	changeDlgLang(preference._highlighting.getHSelf(), "Highlighting", titre, titreMaxSize);
	if (titre[0] != '\0')
	{
		const wchar_t *nameW = wmc->char2wchar(titre, _nativeLangEncoding);
		preference.renameDialogTitle(TEXT("Highlighting"), nameW);
	}

	changeDlgLang(preference._printSettingsDlg.getHSelf(), "Print", titre, titreMaxSize);
	if (titre[0] != '\0')
	{
		const wchar_t *nameW = wmc->char2wchar(titre, _nativeLangEncoding);
		preference.renameDialogTitle(TEXT("Print"), nameW);
	}
	changeDlgLang(preference._settingsDlg.getHSelf(), "MISC", titre, titreMaxSize);
	if (titre[0] != '\0')
	{
		const wchar_t *nameW = wmc->char2wchar(titre, _nativeLangEncoding);
		preference.renameDialogTitle(TEXT("MISC"), nameW);
	}
	changeDlgLang(preference._backupDlg.getHSelf(), "Backup", titre, titreMaxSize);
	if (titre[0] != '\0')
	{
		const wchar_t *nameW = wmc->char2wchar(titre, _nativeLangEncoding);
		preference.renameDialogTitle(TEXT("Backup"), nameW);
	}

	changeDlgLang(preference._autoCompletionDlg.getHSelf(), "AutoCompletion", titre, titreMaxSize);
	if (titre[0] != '\0')
	{
		const wchar_t *nameW = wmc->char2wchar(titre, _nativeLangEncoding);
		preference.renameDialogTitle(TEXT("AutoCompletion"), nameW);
	}

	changeDlgLang(preference._multiInstDlg.getHSelf(), "MultiInstance", titre, titreMaxSize);
	if (titre[0] != '\0')
	{
		const wchar_t *nameW = wmc->char2wchar(titre, _nativeLangEncoding);
		preference.renameDialogTitle(TEXT("MultiInstance"), nameW);
	}

	changeDlgLang(preference._delimiterSettingsDlg.getHSelf(), "Delimiter", titre, titreMaxSize);
	if (titre[0] != '\0')
	{
		const wchar_t *nameW = wmc->char2wchar(titre, _nativeLangEncoding);
		preference.renameDialogTitle(TEXT("Delimiter"), nameW);
	}

	changeDlgLang(preference._settingsOnCloudDlg.getHSelf(), "Cloud", titre, titreMaxSize);
	if (titre[0] != '\0')
	{
		const wchar_t *nameW = wmc->char2wchar(titre, _nativeLangEncoding);
		preference.renameDialogTitle(TEXT("Cloud"), nameW);
	}

	changeDlgLang(preference._searchEngineDlg.getHSelf(), "SearchEngine", titre, titreMaxSize);
	if (titre[0] != '\0')
	{
		const wchar_t *nameW = wmc->char2wchar(titre, _nativeLangEncoding);
		preference.renameDialogTitle(TEXT("SearchEngine"), nameW);
	}

	preference.setListSelection(currentSel);
}

void NativeLangSpeaker::changeShortcutLang()
{
	if (!_nativeLangA) return;

	NppParameters * pNppParam = NppParameters::getInstance();
	vector<CommandShortcut> & mainshortcuts = pNppParam->getUserShortcuts();
	vector<ScintillaKeyMap> & scinshortcuts = pNppParam->getScintillaKeyList();

	TiXmlNodeA *shortcuts = _nativeLangA->FirstChild("Shortcuts");
	if (!shortcuts) return;

	shortcuts = shortcuts->FirstChild("Main");
	if (!shortcuts) return;

	TiXmlNodeA *entriesRoot = shortcuts->FirstChild("Entries");
	if (!entriesRoot) return;

	for (TiXmlNodeA *childNode = entriesRoot->FirstChildElement("Item");
		childNode ;
		childNode = childNode->NextSibling("Item") )
	{
		TiXmlElementA *element = childNode->ToElement();
		int index, id;
		if (element->Attribute("index", &index) && element->Attribute("id", &id))
		{
			if (index > -1 && static_cast<size_t>(index) < mainshortcuts.size()) //valid index only
			{
				const char *name = element->Attribute("name");
				CommandShortcut & csc = mainshortcuts[index];
				if (csc.getID() == (unsigned long)id) 
				{
					WcharMbcsConvertor *wmc = WcharMbcsConvertor::getInstance();
					const wchar_t * nameW = wmc->char2wchar(name, _nativeLangEncoding);
					csc.setName(nameW);
				}
			}
		}
	}

	//Scintilla
	shortcuts = _nativeLangA->FirstChild("Shortcuts");
	if (!shortcuts) return;

	shortcuts = shortcuts->FirstChild("Scintilla");
	if (!shortcuts) return;

	entriesRoot = shortcuts->FirstChild("Entries");
	if (!entriesRoot) return;

	for (TiXmlNodeA *childNode = entriesRoot->FirstChildElement("Item");
		childNode ;
		childNode = childNode->NextSibling("Item") )
	{
		TiXmlElementA *element = childNode->ToElement();
		int index;
		if (element->Attribute("index", &index))
		{
			if (index > -1 && static_cast<size_t>(index) < scinshortcuts.size()) //valid index only
			{
				const char *name = element->Attribute("name");
				ScintillaKeyMap & skm = scinshortcuts[index];

				WcharMbcsConvertor *wmc = WcharMbcsConvertor::getInstance();
				const wchar_t * nameW = wmc->char2wchar(name, _nativeLangEncoding);
				skm.setName(nameW);
			}
		}
	}

}

generic_string NativeLangSpeaker::getShortcutMapperLangStr(const char *nodeName, const TCHAR *defaultStr) const
{
	if (!_nativeLangA) return defaultStr;

	TiXmlNodeA *targetNode = _nativeLangA->FirstChild("Dialog");
	if (!targetNode) return defaultStr;

	targetNode = targetNode->FirstChild("ShortcutMapper");
	if (!targetNode) return defaultStr;

	targetNode = targetNode->FirstChild(nodeName);
	if (!targetNode) return defaultStr;

	const char *name = (targetNode->ToElement())->Attribute("name");
	if (name && name[0])
	{
		WcharMbcsConvertor *wmc = WcharMbcsConvertor::getInstance();
		return wmc->char2wchar(name, _nativeLangEncoding);
	}

	return defaultStr;
}


TiXmlNodeA * NativeLangSpeaker::searchDlgNode(TiXmlNodeA *node, const char *dlgTagName)
{
	TiXmlNodeA *dlgNode = node->FirstChild(dlgTagName);
	if (dlgNode) return dlgNode;
	for (TiXmlNodeA *childNode = node->FirstChildElement();
		childNode ;
		childNode = childNode->NextSibling() )
	{
		dlgNode = searchDlgNode(childNode, dlgTagName);
		if (dlgNode) return dlgNode;
	}
	return NULL;
}

bool NativeLangSpeaker::changeDlgLang(HWND hDlg, const char *dlgTagName, char *title, size_t titleMaxSize)
{
	if (title)
		title[0] = '\0';

	if (!_nativeLangA) return false;

	TiXmlNodeA *dlgNode = _nativeLangA->FirstChild("Dialog");
	if (!dlgNode) return false;

	dlgNode = searchDlgNode(dlgNode, dlgTagName);
	if (!dlgNode) return false;

	WcharMbcsConvertor *wmc = WcharMbcsConvertor::getInstance();

	// Set Title
	const char *title2set = (dlgNode->ToElement())->Attribute("title");
	if ((title2set && title2set[0]) && hDlg)
	{
		const wchar_t *nameW = wmc->char2wchar(title2set, _nativeLangEncoding);
		::SetWindowText(hDlg, nameW);

		if (title && titleMaxSize)
			strncpy(title, title2set, titleMaxSize - 1);
	}

	// Set the text of child control
	for (TiXmlNodeA *childNode = dlgNode->FirstChildElement("Item");
		childNode ;
		childNode = childNode->NextSibling("Item") )
	{
		TiXmlElementA *element = childNode->ToElement();
		int id;
		const char *sentinel = element->Attribute("id", &id);
		const char *name = element->Attribute("name");
		if (sentinel && (name && name[0]))
		{
			HWND hItem = ::GetDlgItem(hDlg, id);
			if (hItem)
			{
				const wchar_t *nameW = wmc->char2wchar(name, _nativeLangEncoding);
				::SetWindowText(hItem, nameW);
			}
		}
	}

	// Set the text of child control
	for (TiXmlNodeA *childNode = dlgNode->FirstChildElement("ComboBox");
		childNode;
		childNode = childNode->NextSibling("ComboBox"))
	{
		std::vector<generic_string> comboElms;
		TiXmlElementA *element = childNode->ToElement();
		int id;
		element->Attribute("id", &id);
		HWND hCombo = ::GetDlgItem(hDlg, id);

		if (hCombo)
		{
			for (TiXmlNodeA *gChildNode = childNode->FirstChildElement("Element");
				gChildNode;
				gChildNode = gChildNode->NextSibling("Element"))
			{
				TiXmlElementA *comBoelement = gChildNode->ToElement();
				const char *name = comBoelement->Attribute("name");
				const wchar_t *nameW = wmc->char2wchar(name, _nativeLangEncoding);
				comboElms.push_back(nameW);
			}
		}

		size_t count = ::SendMessage(hCombo, CB_GETCOUNT, 0, 0);
		if (count == comboElms.size())
		{
			// get selected index
			auto selIndex = ::SendMessage(hCombo, CB_GETCURSEL, 0, 0);

			// remove all old items
			::SendMessage(hCombo, CB_RESETCONTENT, 0, 0);

			// add translated entries
			for (const auto& i : comboElms)
			{
				::SendMessage(hCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(i.c_str()));
			}

			// restore selected index
			::SendMessage(hCombo, CB_SETCURSEL, selIndex, 0);
		}
	}
	return true;
}

bool NativeLangSpeaker::getMsgBoxLang(const char *msgBoxTagName, generic_string & title, generic_string & message)
{
	title = TEXT("");
	message = TEXT("");

	if (!_nativeLangA) return false;

	TiXmlNodeA *msgBoxNode = _nativeLangA->FirstChild("MessageBox");
	if (!msgBoxNode) return false;

	msgBoxNode = searchDlgNode(msgBoxNode, msgBoxTagName);
	if (!msgBoxNode) return false;

	WcharMbcsConvertor *wmc = WcharMbcsConvertor::getInstance();

	// Set Title
	const char *titre = (msgBoxNode->ToElement())->Attribute("title");
	const char *msg = (msgBoxNode->ToElement())->Attribute("message");
	if ((titre && titre[0]) && (msg && msg[0]))
	{
		title = wmc->char2wchar(titre, _nativeLangEncoding);
		message = wmc->char2wchar(msg, _nativeLangEncoding);
		return true;
	}
	return false;
}

generic_string NativeLangSpeaker::getFileBrowserLangMenuStr(int cmdID, const TCHAR *defaultStr) const
{
	if (!_nativeLangA) return defaultStr;

	TiXmlNodeA *targetNode = _nativeLangA->FirstChild("FolderAsWorkspace");
	if (!targetNode) return defaultStr;

	targetNode = targetNode->FirstChild("Menus");
	if (!targetNode) return defaultStr;

	const char *name = NULL;
	for (TiXmlNodeA *childNode = targetNode->FirstChildElement("Item");
		childNode;
		childNode = childNode->NextSibling("Item"))
	{
		TiXmlElementA *element = childNode->ToElement();
		int id;
		const char *idStr = element->Attribute("id", &id);

		if (idStr && id == cmdID)
		{
			name = element->Attribute("name");
			break;
		}
	}

	if (name && name[0])
	{
		WcharMbcsConvertor *wmc = WcharMbcsConvertor::getInstance();
		return wmc->char2wchar(name, _nativeLangEncoding);
	}
	return defaultStr;
}

generic_string NativeLangSpeaker::getProjectPanelLangMenuStr(const char * nodeName, int cmdID, const TCHAR *defaultStr) const
{
	if (!_nativeLangA) return defaultStr;

	TiXmlNodeA *targetNode = _nativeLangA->FirstChild("ProjectManager");
	if (!targetNode) return defaultStr;

	targetNode = targetNode->FirstChild("Menus");
	if (!targetNode) return defaultStr;

	targetNode = targetNode->FirstChild(nodeName);
	if (!targetNode) return defaultStr;

	const char *name = NULL;
	for (TiXmlNodeA *childNode = targetNode->FirstChildElement("Item");
		childNode ;
		childNode = childNode->NextSibling("Item") )
	{
		TiXmlElementA *element = childNode->ToElement();
		int id;
		const char *idStr = element->Attribute("id", &id);

		if (idStr && id == cmdID)
		{
			name = element->Attribute("name");
			break;
		}
	}

	if (name && name[0])
	{
		WcharMbcsConvertor *wmc = WcharMbcsConvertor::getInstance();
		return wmc->char2wchar(name, _nativeLangEncoding);
	}
	return defaultStr;
}

generic_string NativeLangSpeaker::getAttrNameStr(const TCHAR *defaultStr, const char *nodeL1Name, const char *nodeL2Name) const
{
	if (!_nativeLangA) return defaultStr;

	TiXmlNodeA *targetNode = _nativeLangA->FirstChild(nodeL1Name);
	if (!targetNode) return defaultStr;
	if (nodeL2Name)
		targetNode = targetNode->FirstChild(nodeL2Name);

	if (!targetNode) return defaultStr;

	const char *name = (targetNode->ToElement())->Attribute("name");
	if (name && name[0])
	{
		WcharMbcsConvertor *wmc = WcharMbcsConvertor::getInstance();
		return wmc->char2wchar(name, _nativeLangEncoding);
	}
	return defaultStr;
}

int NativeLangSpeaker::messageBox(const char *msgBoxTagName, HWND hWnd, const TCHAR *defaultMessage, const TCHAR *defaultTitle, int msgBoxType, int intInfo, const TCHAR *strInfo)
{
	generic_string msg, title;
	if (!getMsgBoxLang(msgBoxTagName, title, msg))
	{
		title = defaultTitle;
		msg = defaultMessage;
	}
	title = stringReplace(title, TEXT("$INT_REPLACE$"), std::to_wstring(intInfo));
	msg = stringReplace(msg, TEXT("$INT_REPLACE$"), std::to_wstring(intInfo));
	if (strInfo)
	{
		title = stringReplace(title, TEXT("$STR_REPLACE$"), strInfo);
		msg = stringReplace(msg, TEXT("$STR_REPLACE$"), strInfo);
	}
	return ::MessageBox(hWnd, msg.c_str(), title.c_str(), msgBoxType);
}