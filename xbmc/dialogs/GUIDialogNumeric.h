#pragma once

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

#include "guilib/GUIDialog.h"

class CGUIDialogNumeric :
      public CGUIDialog
{
public:
  enum INPUT_MODE { INPUT_TIME = 1, INPUT_DATE, INPUT_IP_ADDRESS, INPUT_PASSWORD, INPUT_NUMBER, INPUT_TIME_SECONDS };
  CGUIDialogNumeric(void);
  virtual ~CGUIDialogNumeric(void);
  virtual bool OnMessage(CGUIMessage& message);
  virtual bool OnAction(const CAction &action);
  virtual bool OnBack(int actionID);
  virtual void FrameMove();

  bool IsConfirmed() const;
  bool IsCanceled() const;
  bool IsInputHidden() const { return m_mode == INPUT_PASSWORD; };

  static bool ShowAndVerifyNewPassword(std::string& strNewPassword);
  static int ShowAndVerifyPassword(std::string& strPassword, const std::string& strHeading, int iRetries);
  static bool ShowAndVerifyInput(std::string& strPassword, const std::string& strHeading, bool bGetUserInput);

  void SetHeading(const std::string &strHeading);
  void SetMode(INPUT_MODE mode, void *initial);
  void SetMode(INPUT_MODE mode, const std::string &initial);
  void GetOutput(void *output) const;
  std::string GetOutput() const;

  static bool ShowAndGetTime(SYSTEMTIME &time, const std::string &heading);
  static bool ShowAndGetDate(SYSTEMTIME &date, const std::string &heading);
  static bool ShowAndGetIPAddress(std::string &IPAddress, const std::string &heading);
  static bool ShowAndGetNumber(std::string& strInput, const std::string &strHeading, unsigned int iAutoCloseTimeoutMs = 0);
  static bool ShowAndGetSeconds(std::string& timeString, const std::string &heading);

protected:
  virtual void OnInitWindow();
  virtual void OnDeinitWindow(int nextWindowID);

  void OnNumber(unsigned int num);
  void VerifyDate(bool checkYear);
  void OnNext();
  void OnPrevious();
  void OnBackSpace();
  void OnOK();
  void OnCancel();

  bool m_bConfirmed;
  bool m_bCanceled;

  INPUT_MODE m_mode;                // the current input mode
  SYSTEMTIME m_datetime;            // for time and date modes
  WORD m_ip[4];                     // for ip address mode
  unsigned int m_block;             // for time, date, and IP methods.
  unsigned int m_lastblock;
  bool m_dirty;                     // true if the current block has been changed.
  std::string m_number;              ///< for number or password input
};
