#include <wx/msgdlg.h> //TODO: Remove after testing
#include "ConversationList.h"

ConversationList::ConversationList(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name):
	wxListBox(parent, id, pos, size, 0, nullptr, style, validator, name)
{
}

void ConversationList::OnMessage(wxCommandEvent& event)
{
	event.Skip();
	wxMessageBox("OnMessage()", "ConversationList");
	//TODO: Blink the list element or something...
};