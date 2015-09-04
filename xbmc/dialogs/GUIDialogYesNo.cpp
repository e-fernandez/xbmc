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

#include "GUIDialogYesNo.h"
#include "guilib/GUIWindowManager.h"
#include "input/Key.h"

#define CONTROL_NO_BUTTON 10
#define CONTROL_YES_BUTTON 11

CGUIDialogYesNo::CGUIDialogYesNo(int overrideId /* = -1 */)
    : CGUIDialogBoxBase(overrideId == -1 ? WINDOW_DIALOG_YES_NO : overrideId, "DialogYesNo.xml")
{
  m_bConfirmed = false;
  m_bCanceled = false;
}

CGUIDialogYesNo::~CGUIDialogYesNo()
{
}

bool CGUIDialogYesNo::OnMessage(CGUIMessage& message)
{
  switch ( message.GetMessage() )
  {
  case GUI_MSG_CLICKED:
    {
      int iControl = message.GetSenderId();
      int iAction = message.GetParam1();
      if (1 || ACTION_SELECT_ITEM == iAction)
      {
        if (iControl == CONTROL_NO_BUTTON)
        {
          m_bConfirmed = false;
          Close();
          return true;
        }
        if (iControl == CONTROL_YES_BUTTON)
        {
          m_bConfirmed = true;
          Close();
          return true;
        }
      }
    }
    break;
  }
  return CGUIDialogBoxBase::OnMessage(message);
}

bool CGUIDialogYesNo::OnBack(int actionID)
{
  m_bCanceled = true;
  m_bConfirmed = false;
  return CGUIDialogBoxBase::OnBack(actionID);
}

bool CGUIDialogYesNo::ShowAndGetInput(CVariant heading, CVariant line0, CVariant line1, CVariant line2, bool &bCanceled)
{
  return ShowAndGetInput(heading, line0, line1, line2, bCanceled, "", "", NO_TIMEOUT);
}

bool CGUIDialogYesNo::ShowAndGetInput(CVariant heading, CVariant line0, CVariant line1, CVariant line2, CVariant noLabel /* = "" */, CVariant yesLabel /* = "" */)
{
  bool bDummy(false);
  return ShowAndGetInput(heading, line0, line1, line2, bDummy, noLabel, yesLabel, NO_TIMEOUT);
}

bool CGUIDialogYesNo::ShowAndGetInput(CVariant heading, CVariant line0, CVariant line1, CVariant line2, bool &bCanceled, CVariant noLabel, CVariant yesLabel, unsigned int autoCloseTime)
{
  CGUIDialogYesNo *dialog = (CGUIDialogYesNo *)g_windowManager.GetWindow(WINDOW_DIALOG_YES_NO);
  if (!dialog)
    return false;

  dialog->SetHeading(heading);
  dialog->SetLine(0, line0);
  dialog->SetLine(1, line1);
  dialog->SetLine(2, line2);
  if (autoCloseTime)
    dialog->SetAutoClose(autoCloseTime);
  dialog->SetChoice(0, !noLabel.empty() ? noLabel : 106);
  dialog->SetChoice(1, !yesLabel.empty() ? yesLabel : 107);
  dialog->m_bCanceled = false;
  dialog->Open();

  bCanceled = dialog->m_bCanceled;
  return (dialog->IsConfirmed()) ? true : false;
}

bool CGUIDialogYesNo::ShowAndGetInput(CVariant heading, CVariant text)
{
  bool bDummy(false);
  return ShowAndGetInput(heading, text, "", "", bDummy);
}

bool CGUIDialogYesNo::ShowAndGetInput(CVariant heading, CVariant text, bool &bCanceled, CVariant noLabel /* = "" */, CVariant yesLabel /* = "" */, unsigned int autoCloseTime)
{
  CGUIDialogYesNo *dialog = (CGUIDialogYesNo *)g_windowManager.GetWindow(WINDOW_DIALOG_YES_NO);
  if (!dialog)
    return false;

  dialog->SetHeading(heading);
  dialog->SetText(text);
  if (autoCloseTime)
    dialog->SetAutoClose(autoCloseTime);
  dialog->m_bCanceled = false;
  dialog->SetChoice(0, !noLabel.empty() ? noLabel : 106);
  dialog->SetChoice(1, !yesLabel.empty() ? yesLabel : 107);
  dialog->Open();

  bCanceled = dialog->m_bCanceled;
  return (dialog->IsConfirmed()) ? true : false;
}

int CGUIDialogYesNo::GetDefaultLabelID(int controlId) const
{
  if (controlId == CONTROL_NO_BUTTON)
    return 106;
  else if (controlId == CONTROL_YES_BUTTON)
    return 107;
  return CGUIDialogBoxBase::GetDefaultLabelID(controlId);
}
