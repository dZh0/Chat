#pragma once
#include <wx/listbox.h>

class ConversationList: public wxListBox
{
public:
	ConversationList(
		wxWindow* parent,
		wxWindowID id,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = 0,
		const wxValidator& validator = wxDefaultValidator,
		const wxString& name = wxASCII_STR(wxListBoxNameStr)
	);
	void AddConversation(const wxString& convName);
	void RemoveConversation(const wxString& convName);
};
