#pragma once

#ifndef _MainWindow_H
#define _MainWindow_H

#include <wx/wx.h>
#include <wx/tglbtn.h>
#include <wx/listctrl.h>
#include <wx/clipbrd.h>

#include <cstdlib>
#include <filesystem>

#include "./../json/jFunc.hpp"


class EpisodePanel : public wxPanel {
public:
  EpisodePanel(wxPanel *parent);
  wxPanel *m_parent;

  void OnOpen();
  void OnOpen(wxCommandEvent& event);

  wxListCtrl *epList;
};

class PreviewPanel : public wxPanel {
public:
  PreviewPanel(wxPanel *parent);
  wxPanel *m_parent;

  void OnRefresh(wxCommandEvent& event);
  void OnCheck(wxCommandEvent& event);
  void refreshAnime(bool, bool);
  void switchCategory(int, bool);
  void OnEdit(wxCommandEvent& event);
  void OnLoadEp();

  wxTextCtrl *animeName, *animePath, *animeLink_MAL, *animeLink_Gogo;
  wxString animeNameS;
  wxStaticText *animeStatus, *animeEp;
  wxToggleButton *editButton;
  wxButton *checkButton, *refreshButton;
  wxStaticBitmap *animeLogo;

  EpisodePanel *epPanell;
};

class MainWindow : public wxFrame {
public:
  MainWindow(const wxString &title, int width, int height, json &profile);
  json m_profile, m_active;

  void OnResize(wxSizeEvent& event);

  void LoadPreview(int, bool, bool);
  void LoadPreview(wxCommandEvent &event);

  void OnRefresh(wxCommandEvent &event);
  void OnAdd(wxCommandEvent &event);
  void OnAddManual(wxCommandEvent &event);
  void OnRemove(wxCommandEvent &event);
  void OnUp(wxCommandEvent &event);
  void OnDown(wxCommandEvent &event);

  void OnCheck(wxCommandEvent &event);

  void ImportDir(wxCommandEvent& event);

  void SetDefaultPath(wxCommandEvent &event);

  void loadProfile();
  void loadCategory(const char *);
  void readProfile();
  void writeProfile();

  wxMenuBar *menubar;
  wxMenu *file, *settings;

  wxPanel *listPanel, *buttonPanel;
  wxListBox *animeListBox;

  wxButton *refreshButton, *addButton, *removeButton;
  wxButton *upButton, *downButton;
  wxButton *checkButton;

  PreviewPanel *previewPanel;
};

const int ID_REFRESH_BUTTON = 5;
const int ID_ADD_BUTTON = 6, ID_ADD_MANUAL = 7;
const int ID_REMOVE_BUTTON = 8;
const int ID_UP_BUTTON = 9;
const int ID_DOWN_BUTTON = 10;
const int ID_CHECK_ALL_BUTTON = 11;

const int ID_SET_DEFAULT_PATH = 12;
const int ID_IMPORT_DIR_MENU = 13;

#endif //_MainWindow_H
