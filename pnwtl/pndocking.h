/**
 * @file pndocking.h
 * @brief Docking Windows implementation for Programmers Notepad 2.
 * @author Simon Steele
 * @note Copyright (c) 2002-2003 Simon Steele <s.steele@pnotepad.org>
 *
 * Programmers Notepad 2 : The license file (license.[txt|html]) describes 
 * the conditions under which this source may be modified / distributed.
 */

#ifndef pndocking_h__included
#define pndocking_h__included

#include <DockMisc.h>
#include <DockingFrame.h>
#include <DockingBox.h>
#include <TabDockingBox.h>

// These includes are to enable the state manager stuff.
#include <sstate.h>

/**
 * @brief Customised CWindowStateMgr allowing named configs.
 */
class CPNStateManager : public sstate::CWindowStateMgr
{
	public:	
		void Initialize(const tstring& strMainKey, HWND hWnd, int nDefCmdShow = SW_SHOWNOACTIVATE)
		{
			m_pImpl->SetWindow(hWnd, nDefCmdShow);
			m_strMainKey = strMainKey;
		}

		void Store(const tstring* strKey = NULL)
		{
			sstate::CMainState mstate(strKey ? *strKey : m_strMainKey);
			if(mstate.Store())
			{
				CRegKey key;
				DWORD dwDisposition;
				if(key.Create(mstate.MainKey(), sstate::ctxtMainWindow, REG_NONE,
					REG_OPTION_NON_VOLATILE, KEY_WRITE | KEY_READ, 
					NULL, &dwDisposition) == ERROR_SUCCESS)
					m_pImpl->Store(&mstate, key);
			}
		}

		/**
		 * Need to return a success value as well as allow named configs.
		 */
		bool Restore(const tstring* strKey = NULL)
		{
			sstate::CMainState mstate(strKey ? *strKey : m_strMainKey);
			CRegKey key;
			if(mstate.Restore() && (key.Open(mstate.MainKey(), 
				sstate::ctxtMainWindow, KEY_READ) == ERROR_SUCCESS))
			{
				m_pImpl->Restore(&mstate, key);
				return true;
			}
			else
			{
				m_pImpl->RestoreDefault();
				return false;
			}
		}
};

/**
 * @brief This is the base class for all docking windows in Programmers Notepad 2.
 */
template <class T>
class /*ATL_NO_VTABLE*/ CPNDockingWindow : public dockwins::CBoxedDockingWindowImpl<T,
                CWindow, dockwins::COutlookLikeBoxedDockingWindowTraits >
{
	typedef dockwins::CBoxedDockingWindowImpl<T, CWindow, dockwins::COutlookLikeBoxedDockingWindowTraits> baseClass;
	typedef CPNDockingWindow<T> thisClass;

	public:
		// Prevent Hide from de-activating the current window. This is a bit kludgy.
		virtual bool Hide()
		{
			bool bRes = true;
			HWND hWndActive = ::GetFocus();
			
			if(IsDocking())
			{
				bRes=GetDockingPosition(&m_pos);
				assert(bRes);
				if(bRes)
					bRes=Float(&m_rcUndock, SWP_HIDEWINDOW | SWP_NOACTIVATE); // Added SWP_NOACTIVATE
				assert(bRes);
			}
			else
				m_pos.hdr.hBar=HNONDOCKBAR;

			::SetFocus(hWndActive);

			return (bRes && ShowWindow(SW_HIDE));
		}

		BEGIN_MSG_MAP(thisClass)
			CHAIN_MSG_MAP(baseClass)
		END_MSG_MAP()
};

/**
 * @brief This class combines the Tabbed MDI framework with the docking windows framework
 */
template <class T, 
		  class TBase = CMDIWindow, 		  
		  class TWinTraits = dockwins::CDockingFrameTraits >
class ATL_NO_VTABLE CPNDockingTabbedMDIFrameWindow : 
	public dockwins::CDockingFrameImplBase< T, CTabbedMDIFrameWindowImpl< T , CPNMDIClient, TBase, TWinTraits> ,TWinTraits >
{
public:
	DECLARE_WND_CLASS(_T("CPNDockingTabbedMDIFrameWindow"))
};


#endif //#ifndef pndocking_h__included