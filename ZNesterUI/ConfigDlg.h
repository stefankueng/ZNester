#pragma once
#include <string>
#include <vector>

#include "BaseDialog.h"
class ConfigDlg : public CDialog
{
public:
	explicit ConfigDlg( HWND hParent );
	~ConfigDlg() = default;

	size_t		 m_width;
	size_t		 m_height;
	std::wstring m_binPath;
	std::wstring m_objPath;
	size_t		 m_copies;
	size_t		 m_partDistance;
	size_t		 m_binDistance;
	size_t		 m_rotations;
	bool		 m_useHoles;

protected:
	LRESULT CALLBACK		  dlgFunc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam ) override;
	LRESULT					  doCommand( int id );

	std::vector<std::wstring> askForFiles( const std::vector<std::wstring>& filters );
	void					  enableControls();

private:
	HWND m_hParent;
};
