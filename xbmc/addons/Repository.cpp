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

#include <iterator>
#include "Repository.h"
#include "events/EventLog.h"
#include "events/AddonManagementEvent.h"
#include "addons/AddonDatabase.h"
#include "addons/AddonInstaller.h"
#include "addons/AddonManager.h"
#include "dialogs/GUIDialogYesNo.h"
#include "dialogs/GUIDialogKaiToast.h"
#include "filesystem/File.h"
#include "filesystem/Directory.h"
#include "settings/Settings.h"
#include "utils/log.h"
#include "utils/JobManager.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "utils/Variant.h"
#include "utils/XBMCTinyXML.h"
#include "FileItem.h"
#include "TextureDatabase.h"
#include "URL.h"

using namespace XFILE;
using namespace ADDON;

AddonPtr CRepository::Clone() const
{
  return AddonPtr(new CRepository(*this));
}

CRepository::CRepository(const AddonProps& props) :
  CAddon(props)
{
}

CRepository::CRepository(const cp_extension_t *ext)
  : CAddon(ext)
{
  // read in the other props that we need
  if (ext)
  {
    AddonVersion version("0.0.0");
    AddonPtr addonver;
    if (CAddonMgr::GetInstance().GetAddon("xbmc.addon", addonver))
      version = addonver->Version();
    for (size_t i = 0; i < ext->configuration->num_children; ++i)
    {
      if(ext->configuration->children[i].name &&
         strcmp(ext->configuration->children[i].name, "dir") == 0)
      {
        AddonVersion min_version(CAddonMgr::GetInstance().GetExtValue(&ext->configuration->children[i], "@minversion"));
        if (min_version <= version)
        {
          DirInfo dir;
          dir.version    = min_version;
          dir.checksum   = CAddonMgr::GetInstance().GetExtValue(&ext->configuration->children[i], "checksum");
          dir.compressed = CAddonMgr::GetInstance().GetExtValue(&ext->configuration->children[i], "info@compressed") == "true";
          dir.info       = CAddonMgr::GetInstance().GetExtValue(&ext->configuration->children[i], "info");
          dir.datadir    = CAddonMgr::GetInstance().GetExtValue(&ext->configuration->children[i], "datadir");
          dir.zipped     = CAddonMgr::GetInstance().GetExtValue(&ext->configuration->children[i], "datadir@zip") == "true";
          dir.hashes     = CAddonMgr::GetInstance().GetExtValue(&ext->configuration->children[i], "hashes") == "true";
          m_dirs.push_back(dir);
        }
      }
    }
    // backward compatibility
    if (!CAddonMgr::GetInstance().GetExtValue(ext->configuration, "info").empty())
    {
      DirInfo info;
      info.checksum   = CAddonMgr::GetInstance().GetExtValue(ext->configuration, "checksum");
      info.compressed = CAddonMgr::GetInstance().GetExtValue(ext->configuration, "info@compressed") == "true";
      info.info       = CAddonMgr::GetInstance().GetExtValue(ext->configuration, "info");
      info.datadir    = CAddonMgr::GetInstance().GetExtValue(ext->configuration, "datadir");
      info.zipped     = CAddonMgr::GetInstance().GetExtValue(ext->configuration, "datadir@zip") == "true";
      info.hashes     = CAddonMgr::GetInstance().GetExtValue(ext->configuration, "hashes") == "true";
      m_dirs.push_back(info);
    }
  }
}

CRepository::CRepository(const CRepository &rhs)
  : CAddon(rhs), m_dirs(rhs.m_dirs)
{
}

CRepository::~CRepository()
{
}

std::string CRepository::FetchChecksum(const std::string& url)
{
  CFile file;
  try
  {
    if (file.Open(url))
    {    
      // we intentionally avoid using file.GetLength() for 
      // Transfer-Encoding: chunked servers.
      std::stringstream str;
      char temp[1024];
      int read;
      while ((read=file.Read(temp, sizeof(temp))) > 0)
        str.write(temp, read);
      return str.str();
    }
    return "";
  }
  catch (...)
  {
    return "";
  }
}

std::string CRepository::GetAddonHash(const AddonPtr& addon) const
{
  std::string checksum;
  DirList::const_iterator it;
  for (it = m_dirs.begin();it != m_dirs.end(); ++it)
    if (URIUtils::IsInPath(addon->Path(), it->datadir))
      break;
  if (it != m_dirs.end() && it->hashes)
  {
    checksum = FetchChecksum(addon->Path()+".md5");
    size_t pos = checksum.find_first_of(" \n");
    if (pos != std::string::npos)
      return checksum.substr(0, pos);
  }
  return checksum;
}

#define SET_IF_NOT_EMPTY(x,y) \
  { \
    if (!x.empty()) \
       x = y; \
  }

bool CRepository::Parse(const DirInfo& dir, VECADDONS &result)
{
  std::string file = dir.info;
  if (dir.compressed)
  {
    CURL url(dir.info);
    std::string opts = url.GetProtocolOptions();
    if (!opts.empty())
      opts += "&";
    url.SetProtocolOptions(opts+"Encoding=gzip");
    file = url.Get();
  }

  CXBMCTinyXML doc;
  if (doc.LoadFile(file) && doc.RootElement() &&
      CAddonMgr::GetInstance().AddonsFromRepoXML(doc.RootElement(), result))
  {
    for (IVECADDONS i = result.begin(); i != result.end(); ++i)
    {
      AddonPtr addon = *i;
      if (dir.zipped)
      {
        std::string file = StringUtils::Format("%s/%s-%s.zip", addon->ID().c_str(), addon->ID().c_str(), addon->Version().asString().c_str());
        addon->Props().path = URIUtils::AddFileToFolder(dir.datadir,file);
        SET_IF_NOT_EMPTY(addon->Props().icon,URIUtils::AddFileToFolder(dir.datadir,addon->ID()+"/icon.png"))
        file = StringUtils::Format("%s/changelog-%s.txt", addon->ID().c_str(), addon->Version().asString().c_str());
        SET_IF_NOT_EMPTY(addon->Props().changelog,URIUtils::AddFileToFolder(dir.datadir,file))
        SET_IF_NOT_EMPTY(addon->Props().fanart,URIUtils::AddFileToFolder(dir.datadir,addon->ID()+"/fanart.jpg"))
      }
      else
      {
        addon->Props().path = URIUtils::AddFileToFolder(dir.datadir,addon->ID()+"/");
        SET_IF_NOT_EMPTY(addon->Props().icon,URIUtils::AddFileToFolder(dir.datadir,addon->ID()+"/icon.png"))
        SET_IF_NOT_EMPTY(addon->Props().changelog,URIUtils::AddFileToFolder(dir.datadir,addon->ID()+"/changelog.txt"))
        SET_IF_NOT_EMPTY(addon->Props().fanart,URIUtils::AddFileToFolder(dir.datadir,addon->ID()+"/fanart.jpg"))
      }
    }
    return true;
  }
  return false;
}

void CRepository::OnPostInstall(bool update, bool modal)
{
  // force refresh of addon repositories
  CAddonInstaller::GetInstance().UpdateRepos(true, false, true);
}

void CRepository::OnPostUnInstall()
{
  CAddonDatabase database;
  database.Open();
  database.DeleteRepository(ID());

  // force refresh of addon repositories
  CAddonInstaller::GetInstance().UpdateRepos(true, false, true);
}

CRepositoryUpdateJob::CRepositoryUpdateJob(const VECADDONS &repos)
  : m_repos(repos)
{
}

void MergeAddons(std::map<std::string, AddonPtr> &addons, const VECADDONS &new_addons)
{
  for (VECADDONS::const_iterator it = new_addons.begin(); it != new_addons.end(); ++it)
  {
    std::map<std::string, AddonPtr>::iterator existing = addons.find((*it)->ID());
    if (existing != addons.end())
    { // already got it - replace if we have a newer version
      if (existing->second->Version() < (*it)->Version())
        existing->second = *it;
    }
    else
      addons.insert(make_pair((*it)->ID(), *it));
  }
}

bool CRepositoryUpdateJob::DoWork()
{
  std::map<std::string, AddonPtr> addons;
  for (VECADDONS::const_iterator i = m_repos.begin(); i != m_repos.end(); ++i)
  {
    const RepositoryPtr repo = std::dynamic_pointer_cast<CRepository>(*i);
    VECADDONS newAddons;
    if (GrabAddons(repo, newAddons))
      MergeAddons(addons, newAddons);
  }
  if (addons.empty())
    return true; //Nothing to do

  // check for updates
  CAddonDatabase database;
  database.Open();
  database.BeginMultipleExecute();

  CTextureDatabase textureDB;
  textureDB.Open();
  textureDB.BeginMultipleExecute();
  VECADDONS notifications;
  for (std::map<std::string, AddonPtr>::const_iterator i = addons.begin(); i != addons.end(); ++i)
  {
    // manager told us to feck off
    if (ShouldCancel(0,0))
      break;

    AddonPtr newAddon = i->second;
    bool deps_met = CAddonInstaller::GetInstance().CheckDependencies(newAddon, &database);
    if (!deps_met && newAddon->Props().broken.empty())
      newAddon->Props().broken = "DEPSNOTMET";

    // invalidate the art associated with this item
    if (!newAddon->Props().fanart.empty())
      textureDB.InvalidateCachedTexture(newAddon->Props().fanart);
    if (!newAddon->Props().icon.empty())
      textureDB.InvalidateCachedTexture(newAddon->Props().icon);

    AddonPtr addon;
    CAddonMgr::GetInstance().GetAddon(newAddon->ID(),addon);
    if (addon && newAddon->Version() > addon->Version() &&
        !database.IsAddonBlacklisted(newAddon->ID(),newAddon->Version().asString()) &&
        deps_met)
    {
      if (CSettings::GetInstance().GetInt(CSettings::SETTING_GENERAL_ADDONUPDATES) == AUTO_UPDATES_ON)
      {
        std::string referer;
        if (URIUtils::IsInternetStream(newAddon->Path()))
          referer = StringUtils::Format("Referer=%s-%s.zip",addon->ID().c_str(),addon->Version().asString().c_str());

        if (newAddon->CanInstall(referer))
          CAddonInstaller::GetInstance().Install(addon->ID(), true, referer);
      }
      else
        notifications.push_back(addon);
    }

    // Check if we should mark the add-on as broken.  We may have a newer version
    // of this add-on in the database or installed - if so, we keep it unbroken.
    bool haveNewer = (addon && addon->Version() > newAddon->Version()) ||
                     database.GetAddonVersion(newAddon->ID()) > newAddon->Version();
    if (!haveNewer)
    {
      // if the add-on is installed and has just been marked as broken (but not in the database yet)
      // ask the user whether he wants to disable the add-on
      if (addon && !newAddon->Props().broken.empty() && database.IsAddonBroken(newAddon->ID()).empty())
      {
        std::string line = g_localizeStrings.Get(24096);
        if (newAddon->Props().broken == "DEPSNOTMET")
          line = g_localizeStrings.Get(24104);
        if (CGUIDialogYesNo::ShowAndGetInput(CVariant{newAddon->Name()}, CVariant{line}, CVariant{24097}, CVariant{""}))
          CAddonMgr::GetInstance().DisableAddon(newAddon->ID());

        CEventLog::GetInstance().Add(EventPtr(new CAddonManagementEvent(newAddon, 24096)));
      }
      database.BreakAddon(newAddon->ID(), newAddon->Props().broken);
    }
  }
  database.CommitMultipleExecute();
  textureDB.CommitMultipleExecute();
  MarkFinished();
  if (!notifications.empty() && CSettings::GetInstance().GetBool(CSettings::SETTING_GENERAL_ADDONNOTIFICATIONS))
  {
    if (notifications.size() == 1)
      CGUIDialogKaiToast::QueueNotification(notifications[0]->Icon(),
                                            g_localizeStrings.Get(24061),
                                            notifications[0]->Name(),TOAST_DISPLAY_TIME,false,TOAST_DISPLAY_TIME);
    else
      CGUIDialogKaiToast::QueueNotification("",
                                            g_localizeStrings.Get(24001),
                                            g_localizeStrings.Get(24061),TOAST_DISPLAY_TIME,false,TOAST_DISPLAY_TIME);
  }

  return true;
}

bool CRepositoryUpdateJob::GrabAddons(const RepositoryPtr& repo, VECADDONS& addons)
{
  SetText(StringUtils::Format(g_localizeStrings.Get(24093).c_str(), repo->Name().c_str()));
  const unsigned int total = repo->m_dirs.size() * 2;

  if (ShouldCancel(0, total))
    return false;

  CAddonDatabase database;
  database.Open();
  std::string oldReposum;
  if (!database.GetRepoChecksum(repo->ID(), oldReposum))
    oldReposum = "";

  std::string reposum;
  for (CRepository::DirList::const_iterator it  = repo->m_dirs.begin(); it != repo->m_dirs.end(); ++it)
  {
    if (ShouldCancel(std::distance(repo->m_dirs.cbegin(), it), total))
      return false;
    if (!it->checksum.empty())
    {
      const std::string dirsum = CRepository::FetchChecksum(it->checksum);
      if (dirsum.empty())
      {
        CLog::Log(LOGERROR, "Failed to fetch checksum for directory listing %s for repository %s. ", (*it).info.c_str(), repo->ID().c_str());
        return false;
      }
      reposum += dirsum;
    }
  }

  if (oldReposum != reposum || oldReposum.empty())
  {
    std::map<std::string, AddonPtr> uniqueAddons;
    for (CRepository::DirList::const_iterator it = repo->m_dirs.begin(); it != repo->m_dirs.end(); ++it)
    {
      if (ShouldCancel(repo->m_dirs.size() + std::distance(repo->m_dirs.cbegin(), it), total))
        return false;
      VECADDONS addons;
      if (!CRepository::Parse(*it, addons))
      { //TODO: Hash is invalid and should not be saved, but should we fail?
        //We can still report a partial addon listing.
        CLog::Log(LOGERROR, "Failed to read directory listing %s for repository %s. ", (*it).info.c_str(), repo->ID().c_str());
        return false;
      }
      MergeAddons(uniqueAddons, addons);
    }

    bool add = true;
    if (!repo->Props().libname.empty())
    {
      CFileItemList dummy;
      std::string s = StringUtils::Format("plugin://%s/?action=update", repo->ID().c_str());
      add = CDirectory::GetDirectory(s, dummy);
    }
    if (add)
    {
      for (std::map<std::string, AddonPtr>::const_iterator i = uniqueAddons.begin(); i != uniqueAddons.end(); ++i)
        addons.push_back(i->second);
      database.AddRepository(repo->ID(), addons, reposum, repo->Version());
    }
  }
  else
  {
    CLog::Log(LOGDEBUG, "Checksum for repository %s not changed.", repo->ID().c_str());
    database.GetRepository(repo->ID(), addons);
    database.SetRepoTimestamp(repo->ID(), CDateTime::GetCurrentDateTime().GetAsDBDateTime(), repo->Version());
  }
  SetProgress(total, total);
  return true;
}

