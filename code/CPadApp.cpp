// ------------------------------------------------------------------------
// CPadApp.cpp						(c)1997 Jesse Hall. Bảo lưu mọi quyền.
// ------------------------------------------------------------------------

#include "CPadApp.h"
#include "Pad.h"
#include <Alert.h>
#include <Entry.h>

// ---------------------------------------------------------------- CPadApp
CPadApp::CPadApp( void )
	: BApplication( "application/x-jessehall-pad" ), mPad( NULL ),
			mIgnorePadClosed( false ) {
}

// --------------------------------------------------------------- ~CPadApp
CPadApp::~CPadApp( void ) {
}

// --------------------------------------------------------- AboutRequested
void CPadApp::AboutRequested( void ) {
	BAlert *alert = new BAlert( NULL, "Pad 1.00\nby Jesse Hall", "OK" );
	alert->Go( NULL );
}

// -------------------------------------------------------- MessageReceived
// ĐÂY LÀ HÀM XỬ LÝ TIN NHẮN CỦA ỨNG DỤNG (CPadApp)
void CPadApp::MessageReceived( BMessage *msg ) {
	switch( msg->what ) {
		// Xử lý khi cửa sổ CPad báo đã đóng
		case msg_PadClosed:
			// Nếu chúng ta không chủ động đóng nó (vd: để mở tệp mới),
			// thì hãy thoát ứng dụng (nếu đây là cửa sổ cuối cùng).
			if( !mIgnorePadClosed ) {
				PostMessage( B_QUIT_REQUESTED );
			}
			// Đặt lại cờ
			mIgnorePadClosed = false;
			mPad = NULL;
			break;
		default:
			BApplication::MessageReceived( msg );
			break;
	}
}

// ------------------------------------------------------------- ReadyToRun
void CPadApp::ReadyToRun( void ) {
	// Nếu chưa có pad nào được tạo, hãy tạo pad mặc định
	if( !mPad ) {
		mPad = new CPad;
		mPad->Show();
	}
}

// ----------------------------------------------------------- RefsReceived
void CPadApp::RefsReceived( BMessage *msg ) {
	entry_ref ref;
	uint32	type;
	int32	count;
	
	// Lấy thông tin tham chiếu (ref)
	msg->GetInfo( "refs", &type, &count );
	if( type == B_REF_TYPE ) {

		// Lặp qua các tham chiếu cho đến khi tìm thấy tệp Pad
		bool good = false;
		BEntry *entry = NULL;
		for( int i = 0; i < count && !good; i++ ) {
			if( msg->FindRef( "refs", i, &ref ) == B_OK ) {

				// Tạo một BEntry từ tham chiếu
				entry = new BEntry( &ref );
				if( entry->IsFile() ) {
					good = true;
				} else {
					delete entry;
					entry = NULL;
				}
			}
		}		
		
		// Nếu chúng ta tìm thấy một tệp tốt
		if( good ) {

			// Nếu một pad đã được mở, hãy đóng nó
			if( mPad ) {
				mIgnorePadClosed = true; // Báo cho app biết đây là hành động có chủ đích
				mPad->PostMessage( B_QUIT_REQUESTED, mPad );
			}
	
			// Tạo một pad mới từ tệp
			mPad = new CPad( entry );
			mPad->Show();
		}
	}
}
