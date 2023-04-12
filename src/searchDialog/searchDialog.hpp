#pragma once

#ifndef _SearchDialog_H
#define _SearchDialog_H

#include <wx/wx.h>

#include "./../json/jFunc.hpp"

class SearchDialog : public wxDialog {
public:
  SearchDialog(const wxString &title, int width, int height, std::string *animeTitle, int *mal, std::string *gogo, bool _gogo);
  std::string *m_animeTitle, *m_gogo; int *m_mal; bool b_gogo;

  void OnSearch();
  void OnSearch(wxCommandEvent& event);
  void OnSelect(wxCommandEvent& event);

  json m_animeList;

  wxTextCtrl *search;
  wxListBox *resultListBox;
};

#endif //_SearchDialog_H
