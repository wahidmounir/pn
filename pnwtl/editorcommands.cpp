/**
 * @file editorcommands.cpp
 * @brief Built in text buffer commands
 * @author Simon Steele
 * @note Copyright (c) 2002-2008 Simon Steele - http://untidy.net/
 *
 * Programmer's Notepad 2 : The license file (license.[txt|html]) describes 
 * the conditions under which this source may be modified / distributed.
 */

#include "stdafx.h"
#include "resource.h"
#include "editorcommands.h"
#include "scintillaimpl.h"

namespace Commands
{

void CompressWhitespace(CScintillaImpl& editor);
void ClipboardSwap(CScintillaImpl& editor);
void DuplicateSelection(CScintillaImpl& editor);
void StripTrailingBlanks(CScintillaImpl& editor);
void TabsToSpaces(CScintillaImpl& editor);
void SpacesToTabs(CScintillaImpl& editor);
void TransformUpperCase(CScintillaImpl& editor);
void TransformLowerCase(CScintillaImpl& editor);
void LineMoveDown(CScintillaImpl& editor);
void LineMoveUp(CScintillaImpl& editor);
void LineTranspose(CScintillaImpl& editor);
void LineCopy(CScintillaImpl& editor);
void LineCut(CScintillaImpl& editor);
void LineDelete(CScintillaImpl& editor);
void LineDuplicate(CScintillaImpl& editor);

void GetEditorCommands(std::list<EditorCommand*>& commands)
{
	commands.push_back(new Internal::EditorCommandFn(ID_EDIT_UPPERCASE, TransformUpperCase));
	commands.push_back(new Internal::EditorCommandFn(ID_EDIT_LOWERCASE, TransformLowerCase));
	commands.push_back(new Internal::EditorCommandFn(ID_LINE_MOVELINEDOWN, LineMoveDown));
	commands.push_back(new Internal::EditorCommandFn(ID_LINE_MOVELINEUP, LineMoveUp));
	commands.push_back(new Internal::EditorCommandFn(ID_EDIT_TRANSPOSELINES, LineTranspose));
	commands.push_back(new Internal::EditorCommandFn(ID_EDIT_COPYLINE, LineCopy));
	commands.push_back(new Internal::EditorCommandFn(ID_EDIT_CUTLINE, LineCut));
	commands.push_back(new Internal::EditorCommandFn(ID_EDIT_DELETELINE, LineDelete));
	commands.push_back(new Internal::EditorCommandFn(ID_EDIT_DUPLICATELINE, LineDuplicate));
	commands.push_back(new Internal::EditorCommandFn(ID_EDIT_CLIPBOARDSWAP, ClipboardSwap));
	commands.push_back(new Internal::EditorCommandFn(ID_SELECTION_COMPRESSWHITESPACE, CompressWhitespace));
	commands.push_back(new Internal::EditorCommandFn(ID_SELECTION_DUPLICATE, DuplicateSelection));
	commands.push_back(new Internal::EditorCommandFn(ID_SELECTION_STRIPTRAILING, StripTrailingBlanks));
	commands.push_back(new Internal::EditorCommandFn(ID_TOOLS_CONVERTSPACESTOTABS, SpacesToTabs));
	commands.push_back(new Internal::EditorCommandFn(ID_TOOLS_CONVERTTABSTOSPACES, TabsToSpaces));
}

/**
 * Duplicate the selection
 */
void DuplicateSelection(CScintillaImpl& editor)
{
	int selLength(editor.GetSelLength());
	if (selLength == 0)
	{
		editor.LineDuplicate();
		return;
	}

	std::string sel;
	sel.resize(selLength + 1);
	editor.GetSelText(&sel[0]);
	sel.resize(selLength);
	editor.InsertText(editor.GetSelectionEnd(), sel.c_str());
}

/**
 * Remove trailing whitespace
 */
void StripTrailingBlanks(CScintillaImpl& editor)
{
	SearchOptions opt;
	opt.SetFindText("[ \t]+$");
	opt.SetReplaceText("");
	opt.SetUseRegExp(true);
	opt.SetNoCursorMove(true);
	if (editor.GetSelLength() > 0)
	{
		opt.SetReplaceInSelection(true);
	}

	editor.ReplaceAll(&opt);
}

/**
 * Convert multiple whitespace characters to a single one
 */
void CompressWhitespace(CScintillaImpl& editor)
{
	SearchOptions opt;
	opt.SetFindText("[ \t][ \t]+");
	opt.SetReplaceText(" ");
	opt.SetUseRegExp(true);
	opt.SetNoCursorMove(true);
	opt.SetReplaceInSelection(editor.GetSelLength() != 0);
	
	editor.BeginUndoAction();
	editor.ReplaceAll(&opt);
	editor.EndUndoAction();
}

/**
 * Convert Spaces to Tabs
 */
void SpacesToTabs(CScintillaImpl& editor)
{
	if (editor.GetTabWidth() < 1)
	{
		UNEXPECTED(_T("Tab width must be greater than 0"));
		return;
	}

	TCHAR buf[100];
	_stprintf(buf, _T("^[ ]{%d}"), editor.GetTabWidth());

	// Find all leading groups of spaces, convert to tabs
	SearchOptions options;
	options.SetFindText(buf);
	options.SetReplaceText("\\t");
	options.SetReplaceInSelection(editor.GetSelLength() != 0);
	options.SetUseRegExp(true);
	options.SetNoCursorMove(true);
	
	editor.BeginUndoAction();
	
	// Repeat until we've replaced all occurances
	while ( editor.ReplaceAll( &options ) != 0 );
	
	editor.EndUndoAction();
}

/**
 * Convert Tabs to Spaces
 */
void TabsToSpaces(CScintillaImpl& editor)
{
	if (editor.GetTabWidth() < 1)
	{
		UNEXPECTED(_T("Tab width must be greater than 0"));
		return;
	}

	// Set the replace text to tab width * space:
	std::string replaceText(editor.GetTabWidth(), ' ');

	SearchOptions options;
	options.SetFindText("^\\t");
	options.SetReplaceText(replaceText.c_str());
	options.SetReplaceInSelection( editor.GetSelLength() != 0 );
	options.SetSearchBackwards(false);
	options.SetUseRegExp(true);
	options.SetNoCursorMove(true);
	
	editor.BeginUndoAction();

	// Repeat until we've replaced all occurances
	while ( editor.ReplaceAll( &options ) != 0 );

	editor.EndUndoAction();
}

void ClipboardSwap(CScintillaImpl& editor)
{
	Scintilla::TextRange tr;

	tr.chrg.cpMin = editor.GetSelectionStart();
	tr.chrg.cpMax = editor.GetSelectionEnd();
	if( tr.chrg.cpMax < tr.chrg.cpMin )
		tr.chrg.cpMax = editor.GetLength();
	int length = tr.chrg.cpMax - tr.chrg.cpMin;
	
	std::vector<char> buffer(length + 1);
	buffer[length] = '\0';
	tr.lpstrText = &buffer[0];
	
	editor.GetTextRange(&tr);

	editor.Paste();

	HGLOBAL hData = ::GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, length+1);
	if( hData )
	{
		if( OpenClipboard(GetCurrentEditor()) )
		{
			EmptyClipboard();
			char* pBuf = static_cast<char*>(::GlobalLock(hData));
			memcpy(pBuf, &buffer[0], length + 1);
			::GlobalUnlock(hData);
			::SetClipboardData(CF_TEXT, hData);
			CloseClipboard();
		}
	}
}

void LineDuplicate(CScintillaImpl& editor)
{
	editor.LineDuplicate();
}

void LineDelete(CScintillaImpl& editor)
{
	editor.LineDelete();
}

void LineCut(CScintillaImpl& editor)
{
	editor.LineCut();
}

void LineCopy(CScintillaImpl& editor)
{
	editor.LineCopy();
}

void LineTranspose(CScintillaImpl& editor)
{
	editor.LineTranspose();
}

/**
 * Move the current line up
 */
void LineMoveUp(CScintillaImpl& editor)
{
	if (editor.LineFromPosition(editor.GetCurrentPos()) == 0)
		return;
	
	editor.BeginUndoAction();
	editor.LineTranspose();
	editor.LineUp();
	editor.EndUndoAction();
}

/**
 * Move the current line down
 */
void LineMoveDown(CScintillaImpl& editor)
{
	if (editor.LineFromPosition(editor.GetCurrentPos()) == (editor.GetLineCount() - 1))
		return;
	
	editor.BeginUndoAction();
	editor.LineDown();
	editor.LineTranspose();
	editor.EndUndoAction();
}

void TransformLowerCase(CScintillaImpl& editor)
{
	editor.LowerCase();
}

void TransformUpperCase(CScintillaImpl& editor)
{
	editor.UpperCase();
}

} // namespace Commands