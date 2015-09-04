/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "GUIViewStateMusic.h"
#include "PlayListPlayer.h"
#include "settings/AdvancedSettings.h"
#include "settings/MediaSourceSettings.h"
#include "settings/Settings.h"
#include "FileItem.h"
#include "guilib/WindowIDs.h"
#include "guilib/LocalizeStrings.h"
#include "utils/log.h"
#include "utils/SortUtils.h"
#include "view/ViewStateSettings.h"

#include "filesystem/Directory.h"
#include "filesystem/MusicDatabaseDirectory.h"
#include "filesystem/VideoDatabaseDirectory.h"

using namespace XFILE;
using namespace MUSICDATABASEDIRECTORY;

int CGUIViewStateWindowMusic::GetPlaylist()
{
  //return PLAYLIST_MUSIC_TEMP;
  return PLAYLIST_MUSIC;
}

bool CGUIViewStateWindowMusic::AutoPlayNextItem()
{
  return CSettings::GetInstance().GetBool(CSettings::SETTING_MUSICPLAYER_AUTOPLAYNEXTITEM) &&
         !CSettings::GetInstance().GetBool(CSettings::SETTING_MUSICPLAYER_QUEUEBYDEFAULT);
}

std::string CGUIViewStateWindowMusic::GetLockType()
{
  return "music";
}

std::string CGUIViewStateWindowMusic::GetExtensions()
{
  return g_advancedSettings.GetMusicExtensions();
}

VECSOURCES& CGUIViewStateWindowMusic::GetSources()
{
  AddAddonsSource("audio", g_localizeStrings.Get(1038), "DefaultAddonMusic.png");
  return CGUIViewState::GetSources();
}

CGUIViewStateMusicSearch::CGUIViewStateMusicSearch(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  std::string strTrackLeft=CSettings::GetInstance().GetString(CSettings::SETTING_MUSICFILES_LIBRARYTRACKFORMAT);
  if (strTrackLeft.empty())
    strTrackLeft = CSettings::GetInstance().GetString(CSettings::SETTING_MUSICFILES_TRACKFORMAT);
  std::string strTrackRight=CSettings::GetInstance().GetString(CSettings::SETTING_MUSICFILES_LIBRARYTRACKFORMATRIGHT);
  if (strTrackRight.empty())
    strTrackRight = CSettings::GetInstance().GetString(CSettings::SETTING_MUSICFILES_TRACKFORMATRIGHT);

  std::string strAlbumLeft = g_advancedSettings.m_strMusicLibraryAlbumFormat;
  if (strAlbumLeft.empty())
    strAlbumLeft = "%B"; // album
  std::string strAlbumRight = g_advancedSettings.m_strMusicLibraryAlbumFormatRight;
  if (strAlbumRight.empty())
    strAlbumRight = "%A"; // artist

  SortAttribute sortAttribute = SortAttributeNone;
  if (CSettings::GetInstance().GetBool(CSettings::SETTING_FILELISTS_IGNORETHEWHENSORTING))
    sortAttribute = SortAttributeIgnoreArticle;

  AddSortMethod(SortByTitle, sortAttribute, 556, LABEL_MASKS("%T - %A", "%D", "%L", "%A"));  // Title, Artist, Duration| empty, empty
  SetSortMethod(SortByTitle);

  const CViewState *viewState = CViewStateSettings::GetInstance().Get("musicnavsongs");
  SetViewAsControl(viewState->m_viewMode);
  SetSortOrder(viewState->m_sortDescription.sortOrder);

  LoadViewState(items.GetPath(), WINDOW_MUSIC_NAV);
}

void CGUIViewStateMusicSearch::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV, CViewStateSettings::GetInstance().Get("musicnavsongs"));
}

CGUIViewStateMusicDatabase::CGUIViewStateMusicDatabase(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  CMusicDatabaseDirectory dir;
  NODE_TYPE NodeType=dir.GetDirectoryChildType(items.GetPath());

  std::string strTrackLeft=CSettings::GetInstance().GetString(CSettings::SETTING_MUSICFILES_LIBRARYTRACKFORMAT);
  if (strTrackLeft.empty())
    strTrackLeft = CSettings::GetInstance().GetString(CSettings::SETTING_MUSICFILES_TRACKFORMAT);
  std::string strTrackRight=CSettings::GetInstance().GetString(CSettings::SETTING_MUSICFILES_LIBRARYTRACKFORMATRIGHT);
  if (strTrackRight.empty())
    strTrackRight = CSettings::GetInstance().GetString(CSettings::SETTING_MUSICFILES_TRACKFORMATRIGHT);

  std::string strAlbumLeft = g_advancedSettings.m_strMusicLibraryAlbumFormat;
  if (strAlbumLeft.empty())
    strAlbumLeft = "%B"; // album
  std::string strAlbumRight = g_advancedSettings.m_strMusicLibraryAlbumFormatRight;
  if (strAlbumRight.empty())
    strAlbumRight = "%A"; // artist

  CLog::Log(LOGDEBUG,"Album format left  = [%s]", strAlbumLeft.c_str());
  CLog::Log(LOGDEBUG,"Album format right = [%s]", strAlbumRight.c_str());

  SortAttribute sortAttribute = SortAttributeNone;
  if (CSettings::GetInstance().GetBool(CSettings::SETTING_FILELISTS_IGNORETHEWHENSORTING))
    sortAttribute = SortAttributeIgnoreArticle;

  switch (NodeType)
  {
  case NODE_TYPE_OVERVIEW:
    {
      AddSortMethod(SortByNone, 551, LABEL_MASKS("%F", "", "%L", ""));  // Filename, empty | Foldername, empty
      SetSortMethod(SortByNone);

      SetViewAsControl(DEFAULT_VIEW_LIST);

      SetSortOrder(SortOrderNone);
    }
    break;
  case NODE_TYPE_TOP100:
    {
      AddSortMethod(SortByNone, 551, LABEL_MASKS("%F", "", "%L", ""));  // Filename, empty | Foldername, empty
      SetSortMethod(SortByNone);

      SetViewAsControl(DEFAULT_VIEW_LIST);

      SetSortOrder(SortOrderNone);
    }
    break;
  case NODE_TYPE_GENRE:
    {
      AddSortMethod(SortByGenre, 515, LABEL_MASKS("%F", "", "%G", ""));  // Filename, empty | Genre, empty
      SetSortMethod(SortByGenre);

      SetViewAsControl(DEFAULT_VIEW_LIST);

      SetSortOrder(SortOrderAscending);
    }
    break;
  case NODE_TYPE_YEAR:
    {
      AddSortMethod(SortByLabel, 562, LABEL_MASKS("%F", "", "%Y", ""));  // Filename, empty | Year, empty
      SetSortMethod(SortByLabel);

      SetViewAsControl(DEFAULT_VIEW_LIST);

      SetSortOrder(SortOrderAscending);
    }
    break;
  case NODE_TYPE_ARTIST:
    {
      AddSortMethod(SortByArtist, sortAttribute, 557, LABEL_MASKS("%F", "", "%A", ""));  // Filename, empty | Artist, empty
      AddSortMethod(SortByDateAdded, sortAttribute, 570, LABEL_MASKS("%F", "", "%A", "%a"));  // Filename, empty | Artist, dateAdded
      SetSortMethod(SortByArtist);

      const CViewState *viewState = CViewStateSettings::GetInstance().Get("musicnavartists");
      SetViewAsControl(viewState->m_viewMode);
      SetSortOrder(viewState->m_sortDescription.sortOrder);
    }
    break;
  case NODE_TYPE_ALBUM_COMPILATIONS:
  case NODE_TYPE_ALBUM:
  case NODE_TYPE_YEAR_ALBUM:
    {
      // album
      AddSortMethod(SortByAlbum, sortAttribute, 558, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
      // artist
      AddSortMethod(SortByArtist, sortAttribute, 557, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
      // artist / year
      AddSortMethod(SortByArtistThenYear, sortAttribute, 578, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
      // year
      AddSortMethod(SortByYear, 562, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));
      // album date added
      AddSortMethod(SortByDateAdded, sortAttribute, 570, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined

      const CViewState *viewState = CViewStateSettings::GetInstance().Get("musicnavalbums");
      SetSortMethod(viewState->m_sortDescription);
      SetViewAsControl(viewState->m_viewMode);
      SetSortOrder(viewState->m_sortDescription.sortOrder);
    }
    break;
  case NODE_TYPE_ALBUM_RECENTLY_ADDED:
    {
      AddSortMethod(SortByNone, 552, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
      SetSortMethod(SortByNone);

      SetViewAsControl(CViewStateSettings::GetInstance().Get("musicnavalbums")->m_viewMode);

      SetSortOrder(SortOrderNone);
    }
    break;
  case NODE_TYPE_ALBUM_RECENTLY_ADDED_SONGS:
    {
      AddSortMethod(SortByNone, 552, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined | empty, empty
      SetSortMethod(SortByNone);

      SetViewAsControl(CViewStateSettings::GetInstance().Get("musicnavsongs")->m_viewMode);

      SetSortOrder(SortOrderNone);
    }
    break;
  case NODE_TYPE_ALBUM_RECENTLY_PLAYED:
    {
      AddSortMethod(SortByNone, 568, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
      SetSortMethod(SortByNone);

      SetViewAsControl(CViewStateSettings::GetInstance().Get("musicnavalbums")->m_viewMode);

      SetSortOrder(SortOrderNone);
    }
    break;
  case NODE_TYPE_ALBUM_RECENTLY_PLAYED_SONGS:
    {
      AddSortMethod(SortByNone, 568, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined | empty, empty
      SetSortMethod(SortByNone);

      SetViewAsControl(CViewStateSettings::GetInstance().Get("musicnavalbums")->m_viewMode);

      SetSortOrder(SortOrderNone);
    }
    break;
  case NODE_TYPE_ALBUM_TOP100:
    {
      AddSortMethod(SortByNone, 551, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
      SetSortMethod(SortByNone);

      SetViewAsControl(DEFAULT_VIEW_LIST);
      SetSortOrder(SortOrderNone);
    }
    break;
  case NODE_TYPE_SINGLES:
    {
      AddSortMethod(SortByArtist, sortAttribute, 557, LABEL_MASKS("%A - %T", "%D"));  // Artist, Title, Duration| empty, empty
      AddSortMethod(SortByArtistThenYear, sortAttribute, 578, LABEL_MASKS("%A - %T", "%D"));  // Artist, Title, Duration| empty, empty
      AddSortMethod(SortByTitle, sortAttribute, 556, LABEL_MASKS("%T - %A", "%D"));  // Title, Artist, Duration| empty, empty
      AddSortMethod(SortByLabel, sortAttribute, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
      AddSortMethod(SortByTime, 180, LABEL_MASKS("%T - %A", "%D"));  // Titel, Artist, Duration| empty, empty
      AddSortMethod(SortByRating, 563, LABEL_MASKS("%T - %A", "%R"));  // Title - Artist, Rating
      AddSortMethod(SortByDateAdded, 570, LABEL_MASKS("%A - %T", "%a"));  // Title - Artist, DateAdded | empty, empty

      const CViewState *viewState = CViewStateSettings::GetInstance().Get("musicnavsongs");
      SetSortMethod(viewState->m_sortDescription);
      SetViewAsControl(viewState->m_viewMode);
      SetSortOrder(viewState->m_sortDescription.sortOrder);
    }
    break;
  case NODE_TYPE_ALBUM_COMPILATIONS_SONGS:
  case NODE_TYPE_ALBUM_TOP100_SONGS:
  case NODE_TYPE_YEAR_SONG:
  case NODE_TYPE_SONG:
    {
      AddSortMethod(SortByTrackNumber, 554, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined| empty, empty
      AddSortMethod(SortByTitle, sortAttribute, 556, LABEL_MASKS("%T - %A", "%D"));  // Title, Artist, Duration| empty, empty
      AddSortMethod(SortByAlbum, sortAttribute, 558, LABEL_MASKS("%B - %T - %A", "%D"));  // Album, Title, Artist, Duration| empty, empty
      AddSortMethod(SortByArtist, sortAttribute, 557, LABEL_MASKS("%A - %T", "%D"));  // Artist, Title, Duration| empty, empty
      AddSortMethod(SortByArtistThenYear, sortAttribute, 578, LABEL_MASKS("%A - %T", "%D"));  // Artist, Title, Duration| empty, empty
      AddSortMethod(SortByLabel, sortAttribute, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
      AddSortMethod(SortByTime, 180, LABEL_MASKS("%T - %A", "%D"));  // Titel, Artist, Duration| empty, empty
      AddSortMethod(SortByRating, 563, LABEL_MASKS("%T - %A", "%R"));  // Title - Artist, Rating
      AddSortMethod(SortByYear, 562, LABEL_MASKS("%T - %A", "%Y")); // Title, Artist, Year
      AddSortMethod(SortByDateAdded, 570, LABEL_MASKS("%A - %T", "%a"));  // Title - Artist, DateAdded | empty, empty

      const CViewState *viewState = CViewStateSettings::GetInstance().Get("musicnavsongs");
      // the "All Albums" entries always default to SortByAlbum as this is most logical - user can always
      // change it and the change will be saved for this particular path
      if (dir.IsAllItem(items.GetPath()))
        SetSortMethod(SortByAlbum);
      else
        SetSortMethod(viewState->m_sortDescription);

      AddSortMethod(SortByPlaycount, 567, LABEL_MASKS("%T - %A", "%V"));  // Titel - Artist, PlayCount

      SetViewAsControl(viewState->m_viewMode);
      SetSortOrder(viewState->m_sortDescription.sortOrder);
    }
    break;
  case NODE_TYPE_SONG_TOP100:
    {
      AddSortMethod(SortByNone, 576, LABEL_MASKS("%T - %A", "%V"));
      SetSortMethod(SortByPlaycount);

      SetViewAsControl(CViewStateSettings::GetInstance().Get("musicnavsongs")->m_viewMode);

      SetSortOrder(SortOrderNone);
    }
    break;
  default:
    break;
  }

  LoadViewState(items.GetPath(), WINDOW_MUSIC_NAV);
}

void CGUIViewStateMusicDatabase::SaveViewState()
{
  CMusicDatabaseDirectory dir;
  NODE_TYPE NodeType=dir.GetDirectoryChildType(m_items.GetPath());

  switch (NodeType)
  {
    case NODE_TYPE_ARTIST:
      SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV, CViewStateSettings::GetInstance().Get("musicnavartists"));
      break;
    case NODE_TYPE_ALBUM_COMPILATIONS:
    case NODE_TYPE_ALBUM:
    case NODE_TYPE_YEAR_ALBUM:
      SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV, CViewStateSettings::GetInstance().Get("musicnavalbums"));
      break;
    case NODE_TYPE_ALBUM_RECENTLY_ADDED:
    case NODE_TYPE_ALBUM_TOP100:
    case NODE_TYPE_ALBUM_RECENTLY_PLAYED:
      SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV);
      break;
    case NODE_TYPE_SINGLES:
    case NODE_TYPE_ALBUM_COMPILATIONS_SONGS:
    case NODE_TYPE_SONG:
    case NODE_TYPE_YEAR_SONG:
      SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV, CViewStateSettings::GetInstance().Get("musicnavsongs"));
      break;
    case NODE_TYPE_ALBUM_RECENTLY_PLAYED_SONGS:
    case NODE_TYPE_ALBUM_RECENTLY_ADDED_SONGS:
    case NODE_TYPE_SONG_TOP100:
      SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV);
      break;
    default:
      SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV);
      break;
  }
}

CGUIViewStateMusicSmartPlaylist::CGUIViewStateMusicSmartPlaylist(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  SortAttribute sortAttribute = SortAttributeNone;
  if (CSettings::GetInstance().GetBool(CSettings::SETTING_FILELISTS_IGNORETHEWHENSORTING))
    sortAttribute = SortAttributeIgnoreArticle;

  if (items.GetContent() == "songs" || items.GetContent() == "mixed") 
  {
    std::string strTrackLeft=CSettings::GetInstance().GetString(CSettings::SETTING_MUSICFILES_TRACKFORMAT);
    std::string strTrackRight=CSettings::GetInstance().GetString(CSettings::SETTING_MUSICFILES_TRACKFORMATRIGHT);

    AddSortMethod(SortByTrackNumber, 554, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined| empty, empty
    AddSortMethod(SortByTitle, sortAttribute, 556, LABEL_MASKS("%T - %A", "%D"));  // Title, Artist, Duration| empty, empty
    AddSortMethod(SortByAlbum, sortAttribute, 558, LABEL_MASKS("%B - %T - %A", "%D"));  // Album, Titel, Artist, Duration| empty, empty
    AddSortMethod(SortByArtist, sortAttribute, 557, LABEL_MASKS("%A - %T", "%D"));  // Artist, Titel, Duration| empty, empty
    AddSortMethod(SortByArtistThenYear, sortAttribute, 578, LABEL_MASKS("%A - %T", "%D"));  // Artist, Titel, Duration| empty, empty
    AddSortMethod(SortByLabel, sortAttribute, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
    AddSortMethod(SortByTime, 180, LABEL_MASKS("%T - %A", "%D"));  // Titel, Artist, Duration| empty, empty
    AddSortMethod(SortByRating, 563, LABEL_MASKS("%T - %A", "%R"));  // Titel, Artist, Rating| empty, empty
    AddPlaylistOrder(items, LABEL_MASKS(strTrackLeft, strTrackRight));

    SetViewAsControl(CViewStateSettings::GetInstance().Get("musicnavsongs")->m_viewMode);
  } 
  else if (items.GetContent() == "albums") 
  {
    std::string strAlbumLeft = g_advancedSettings.m_strMusicLibraryAlbumFormat;
    if (strAlbumLeft.empty())
      strAlbumLeft = "%B"; // album
    std::string strAlbumRight = g_advancedSettings.m_strMusicLibraryAlbumFormatRight;
    if (strAlbumRight.empty())
      strAlbumRight = "%A"; // artist

    // album
    AddSortMethod(SortByAlbum, sortAttribute, 558, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
    // artist
    AddSortMethod(SortByArtist, sortAttribute, 557, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
    // artist / year
    AddSortMethod(SortByArtistThenYear, sortAttribute, 578, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));  // Filename, empty | Userdefined, Userdefined
    // year
    AddSortMethod(SortByYear, 562, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));

    AddPlaylistOrder(items, LABEL_MASKS("%F", "", strAlbumLeft, strAlbumRight));

    SetViewAsControl(CViewStateSettings::GetInstance().Get("musicnavalbums")->m_viewMode);
  } 
  else 
  {
    CLog::Log(LOGERROR,"Music Smart Playlist must be one of songs, mixed or albums");
  }
  
  LoadViewState(items.GetPath(), WINDOW_MUSIC_NAV);
}

void CGUIViewStateMusicSmartPlaylist::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV, CViewStateSettings::GetInstance().Get("musicnavsongs"));
}

CGUIViewStateMusicPlaylist::CGUIViewStateMusicPlaylist(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  SortAttribute sortAttribute = SortAttributeNone;
  if (CSettings::GetInstance().GetBool(CSettings::SETTING_FILELISTS_IGNORETHEWHENSORTING))
    sortAttribute = SortAttributeIgnoreArticle;
  
  std::string strTrackLeft=CSettings::GetInstance().GetString(CSettings::SETTING_MUSICFILES_TRACKFORMAT);
  std::string strTrackRight=CSettings::GetInstance().GetString(CSettings::SETTING_MUSICFILES_TRACKFORMATRIGHT);

  AddSortMethod(SortByPlaylistOrder, 559, LABEL_MASKS(strTrackLeft, strTrackRight));
  AddSortMethod(SortByTrackNumber, 554, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined| empty, empty
  AddSortMethod(SortByTitle, sortAttribute, 556, LABEL_MASKS("%T - %A", "%D"));  // Title, Artist, Duration| empty, empty
  AddSortMethod(SortByAlbum, sortAttribute, 558, LABEL_MASKS("%B - %T - %A", "%D"));  // Album, Titel, Artist, Duration| empty, empty
  AddSortMethod(SortByArtist, sortAttribute, 557, LABEL_MASKS("%A - %T", "%D"));  // Artist, Titel, Duration| empty, empty
  AddSortMethod(SortByArtistThenYear, sortAttribute, 578, LABEL_MASKS("%A - %T", "%D"));  // Artist, Titel, Duration| empty, empty
  AddSortMethod(SortByLabel, sortAttribute, 551, LABEL_MASKS(strTrackLeft, strTrackRight));
  AddSortMethod(SortByTime, 180, LABEL_MASKS("%T - %A", "%D"));  // Titel, Artist, Duration| empty, empty
  AddSortMethod(SortByRating, 563, LABEL_MASKS("%T - %A", "%R"));  // Titel, Artist, Rating| empty, empty

  SetSortMethod(SortByPlaylistOrder);
  const CViewState *viewState = CViewStateSettings::GetInstance().Get("musicfiles");
  SetViewAsControl(viewState->m_viewMode);
  SetSortOrder(viewState->m_sortDescription.sortOrder);

  LoadViewState(items.GetPath(), WINDOW_MUSIC_FILES);
}

void CGUIViewStateMusicPlaylist::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_FILES);
}

CGUIViewStateWindowMusicNav::CGUIViewStateWindowMusicNav(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  SortAttribute sortAttribute = SortAttributeNone;
  if (CSettings::GetInstance().GetBool(CSettings::SETTING_FILELISTS_IGNORETHEWHENSORTING))
    sortAttribute = SortAttributeIgnoreArticle;

  if (items.IsVirtualDirectoryRoot())
  {
    AddSortMethod(SortByNone, 551, LABEL_MASKS("%F", "%I", "%L", ""));  // Filename, Size | Foldername, empty
    SetSortMethod(SortByNone);

    SetViewAsControl(DEFAULT_VIEW_LIST);

    SetSortOrder(SortOrderNone);
  }
  else
  {
    if (items.IsVideoDb() && items.Size() > (CSettings::GetInstance().GetBool(CSettings::SETTING_FILELISTS_SHOWPARENTDIRITEMS)?1:0))
    {
      XFILE::VIDEODATABASEDIRECTORY::CQueryParams params;
      XFILE::CVideoDatabaseDirectory::GetQueryParams(items[CSettings::GetInstance().GetBool(CSettings::SETTING_FILELISTS_SHOWPARENTDIRITEMS)?1:0]->GetPath(),params);
      if (params.GetMVideoId() != -1)
      {
        AddSortMethod(SortByLabel, sortAttribute, 551, LABEL_MASKS("%T", "%Y"));  // Filename, Duration | Foldername, empty
        AddSortMethod(SortByYear, 562, LABEL_MASKS("%T", "%Y"));
        AddSortMethod(SortByArtist, sortAttribute, 557, LABEL_MASKS("%A - %T", "%Y"));
        AddSortMethod(SortByArtistThenYear, sortAttribute, 578, LABEL_MASKS("%A - %T", "%Y"));
        AddSortMethod(SortByAlbum, sortAttribute, 558, LABEL_MASKS("%B - %T", "%Y"));
        
        std::string strTrackLeft=CSettings::GetInstance().GetString(CSettings::SETTING_MUSICFILES_TRACKFORMAT);
        std::string strTrackRight=CSettings::GetInstance().GetString(CSettings::SETTING_MUSICFILES_TRACKFORMATRIGHT);
        AddSortMethod(SortByTrackNumber, 554, LABEL_MASKS(strTrackLeft, strTrackRight));  // Userdefined, Userdefined| empty, empty
      }
      else
      {
        AddSortMethod(SortByLabel, 551, LABEL_MASKS("%F", "%D", "%L", ""));  // Filename, Duration | Foldername, empty
        SetSortMethod(SortByLabel);
      }
    }
    else
    {
      AddSortMethod(SortByLabel, 551, LABEL_MASKS("%F", "%D", "%L", ""));  // Filename, Duration | Foldername, empty
      SetSortMethod(SortByLabel);
    }
    SetViewAsControl(DEFAULT_VIEW_LIST);

    SetSortOrder(SortOrderAscending);
  }
  LoadViewState(items.GetPath(), WINDOW_MUSIC_NAV);
}

void CGUIViewStateWindowMusicNav::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_NAV);
}

void CGUIViewStateWindowMusicNav::AddOnlineShares()
{
  if (!g_advancedSettings.m_bVirtualShares)
    return;

  VECSOURCES *musicSources = CMediaSourceSettings::GetInstance().GetSources("music");

  for (int i = 0; i < (int)musicSources->size(); ++i)
  {
    CMediaSource share = musicSources->at(i);
  }
}

VECSOURCES& CGUIViewStateWindowMusicNav::GetSources()
{
  //  Setup shares we want to have
  m_sources.clear();
  CFileItemList items;

  CDirectory::GetDirectory("library://music/", items, "");
  for (int i=0; i<items.Size(); ++i)
  {
    CFileItemPtr item=items[i];
    CMediaSource share;
    share.strName = item->GetLabel();
    share.strPath = item->GetPath();
    share.m_strThumbnailImage = item->GetIconImage();
    share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
    m_sources.push_back(share);
  }

  AddOnlineShares();

  return CGUIViewStateWindowMusic::GetSources();
}

CGUIViewStateWindowMusicSongs::CGUIViewStateWindowMusicSongs(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  if (items.IsVirtualDirectoryRoot())
  {
    AddSortMethod(SortByLabel, 551, LABEL_MASKS()); // Preformated
    AddSortMethod(SortByDriveType, 564, LABEL_MASKS()); // Preformated
    SetSortMethod(SortByLabel);

    SetViewAsControl(DEFAULT_VIEW_LIST);

    SetSortOrder(SortOrderAscending);
  }
  else if (items.GetPath() == "special://musicplaylists/")
  { // playlists list sorts by label only, ignoring folders
    AddSortMethod(SortByLabel, SortAttributeIgnoreFolders, 551, LABEL_MASKS("%F", "%D", "%L", ""));  // Filename, Duration | Foldername, empty
    SetSortMethod(SortByLabel);
  }
  else
  {
    std::string strTrackLeft=CSettings::GetInstance().GetString(CSettings::SETTING_MUSICFILES_TRACKFORMAT);
    std::string strTrackRight=CSettings::GetInstance().GetString(CSettings::SETTING_MUSICFILES_TRACKFORMATRIGHT);

    AddSortMethod(SortByLabel, 551, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""),  // Userdefined, Userdefined | FolderName, empty
      CSettings::GetInstance().GetBool(CSettings::SETTING_FILELISTS_IGNORETHEWHENSORTING) ? SortAttributeIgnoreArticle : SortAttributeNone);
    AddSortMethod(SortBySize, 553, LABEL_MASKS(strTrackLeft, "%I", "%L", "%I"));  // Userdefined, Size | FolderName, Size
    AddSortMethod(SortByBitrate, 623, LABEL_MASKS(strTrackLeft, "%X", "%L", "%X"));  // Userdefined, Bitrate | FolderName, Bitrate  
    AddSortMethod(SortByDate, 552, LABEL_MASKS(strTrackLeft, "%J", "%L", "%J"));  // Userdefined, Date | FolderName, Date
    AddSortMethod(SortByFile, 561, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""));  // Userdefined, Userdefined | FolderName, empty
    AddSortMethod(SortByListeners, 20455,LABEL_MASKS(strTrackLeft, "%W", "%L", "%W"));
    
    const CViewState *viewState = CViewStateSettings::GetInstance().Get("musicfiles");
    SetSortMethod(viewState->m_sortDescription);
    SetViewAsControl(viewState->m_viewMode);
    SetSortOrder(viewState->m_sortDescription.sortOrder);
  }
  LoadViewState(items.GetPath(), WINDOW_MUSIC_FILES);
}

void CGUIViewStateWindowMusicSongs::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_FILES, CViewStateSettings::GetInstance().Get("musicfiles"));
}

VECSOURCES& CGUIViewStateWindowMusicSongs::GetSources()
{
  VECSOURCES *musicSources = CMediaSourceSettings::GetInstance().GetSources("music");
  AddOrReplace(*musicSources, CGUIViewStateWindowMusic::GetSources());
  return *musicSources;
}

CGUIViewStateWindowMusicPlaylist::CGUIViewStateWindowMusicPlaylist(const CFileItemList& items) : CGUIViewStateWindowMusic(items)
{
  std::string strTrackLeft=CSettings::GetInstance().GetString(CSettings::SETTING_MUSICFILES_NOWPLAYINGTRACKFORMAT);
  if (strTrackLeft.empty())
    strTrackLeft = CSettings::GetInstance().GetString(CSettings::SETTING_MUSICFILES_TRACKFORMAT);
  std::string strTrackRight=CSettings::GetInstance().GetString(CSettings::SETTING_MUSICFILES_NOWPLAYINGTRACKFORMATRIGHT);
  if (strTrackRight.empty())
    strTrackRight = CSettings::GetInstance().GetString(CSettings::SETTING_MUSICFILES_TRACKFORMATRIGHT);

  AddSortMethod(SortByNone, 551, LABEL_MASKS(strTrackLeft, strTrackRight, "%L", ""));  // Userdefined, Userdefined | FolderName, empty
  SetSortMethod(SortByNone);

  SetViewAsControl(DEFAULT_VIEW_LIST);

  SetSortOrder(SortOrderNone);

  LoadViewState(items.GetPath(), WINDOW_MUSIC_PLAYLIST);
}

void CGUIViewStateWindowMusicPlaylist::SaveViewState()
{
  SaveViewToDb(m_items.GetPath(), WINDOW_MUSIC_PLAYLIST);
}

int CGUIViewStateWindowMusicPlaylist::GetPlaylist()
{
  return PLAYLIST_MUSIC;
}

bool CGUIViewStateWindowMusicPlaylist::AutoPlayNextItem()
{
  return false;
}

bool CGUIViewStateWindowMusicPlaylist::HideParentDirItems()
{
  return true;
}

VECSOURCES& CGUIViewStateWindowMusicPlaylist::GetSources()
{
  m_sources.clear();
  //  Playlist share
  CMediaSource share;
  share.strPath = "playlistmusic://";
  share.m_iDriveType = CMediaSource::SOURCE_TYPE_LOCAL;
  m_sources.push_back(share);

  // CGUIViewState::GetSources would add music plugins
  return m_sources;
}
