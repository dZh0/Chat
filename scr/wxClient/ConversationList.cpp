#include "ConversationList.h"

ConversationList::ConversationList(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name):
	wxListBox(parent, id, pos, size, 0, nullptr, style, validator, name)
{}

void ConversationList::AddConversation(const wxString & convName)
{
	Append(convName);
};

void ConversationList::RemoveConversation(const wxString& convName)
{
	Delete(0); //should find the proper list item
}