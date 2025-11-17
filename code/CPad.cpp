// ========================================================================
// CPad.cpp							(c)1997 Jesse Hall. Bảo lưu mọi quyền.
// ========================================================================

#include "CLine.h"
#include "CPad.h"
#include "Pad.h" // Chứa định nghĩa msg_Open, msg_Save...
#include <stdio.h>
#include <File.h>
#include <Alert.h>
#include <MenuItem.h>
#include <Button.h>
#include <string.h>
#include "CPadApp.h"

// --- KHAI BÁO HÀM MỚI ---
// (Hàm này cũng cần được thêm vào CPad.h khi bạn tạo nó)
void CPad::OpenFile( void ) {
	// Đây là nơi bạn sẽ triển khai BFilePanel để mở tệp
	// Hiện tại, chúng ta chỉ hiển thị một thông báo
	BAlert *alert = new BAlert( NULL, "Chức năng 'Open' chưa được triển khai!", "OK" );
	alert->Go();
}


// ------------------------------------------------------------------- CPad
CPad::CPad( BEntry *entry )
	// ... (Nội dung hàm tạo giữ nguyên, không thay đổi) ...
	: BWindow( BRect( 200.0, 100.0, 500.0, 300.0 ), "Pad", B_TITLED_WINDOW,
			B_NOT_ZOOMABLE ), mEntry( NULL ), mScroll( NULL ),
			mFooter( NULL ), mMenu( NULL ), mCurPage( 0 ) {

	// Tạo giao diện
	SetupWindow();
	
	// Đặt giới hạn kích thước của cửa sổ
	float minW, minH, maxW, maxH;
	GetSizeLimits( &minW, &maxW, &minH, &maxH );
	minW = 225.0;
	minH = 100.0;
	SetSizeLimits( minW, maxW, minH, maxH );

	// Đặt mEntry
	if( entry ) {
		mEntry = entry;
	} else {
		mEntry = GetDefaultPad();
	}
	
	// Thử đọc tệp pad
	bool success = true;
	if( !mEntry->Exists() ) {
		success = false;
	} else {
		BFile file( mEntry, B_READ_ONLY );

		// Tìm xem đây là phiên bản nào
		uint32 info[ 3 ];
		if( file.ReadAttr( kInfoAttr, B_RAW_TYPE, 0, &info,
				sizeof( info ) ) == sizeof( info ) ) {

			// Gọi hàm thích hợp dựa trên phiên bản
			switch( info[ 0 ] ) {
				case 0x0100:		// Phiên bản 1.0
					success = ReadPad100( file );
					break;
				default:
					success = false;
					break;
			}
		} else {
			success = false;
		}
		
		// Tệp tồn tại nhưng không thể đọc được
		if( !success ) {
			BAlert *alert = new BAlert( NULL, "The pad file is corrupted!",
					"Damn", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT );
			alert->Go();
		}
	}
	
	// Nếu đến giờ vẫn chưa mở được pad, hãy tạo một pad trống
	if( !success ) {
		NewPad();
	}
}


// ------------------------------------------------------------------ ~CPad
// ... (Hàm hủy giữ nguyên) ...
CPad::~CPad( void ) {
	// Xóa entry tệp pad
	if( mEntry ) {
		delete mEntry;
	}
	
	// Hủy tất cả các trang
	try {
		RemoveChild( mPages.ItemAt( mCurPage - 1 ) );
	} catch( ... ) {}
	while( !mPages.IsEmpty() ) {
		delete mPages.RemoveItemAt( 0 );
	}
}


// -------------------------------------------------------- MessageReceived
// *** ĐÂY LÀ HÀM ĐÃ ĐƯỢC SỬA ***
void CPad::MessageReceived( BMessage *msg ) {
	switch( msg->what ) {
		// Thêm các case từ menu File
		case msg_Open:
			OpenFile(); // Gọi hàm mở tệp
			break;	
		case msg_Save:
			SaveRequested(); // Gọi hàm lưu tệp
			break;
			
		// Các case cũ cho menu Note
		case msg_Next:
			NextPage();
			break;
		case msg_Prev:
			PrevPage();
			break;
		case msg_New:
			NewPage();
			break;
		case msg_Del:
			DelPage();
			break;
			
		// Case mặc định
		default:
			BWindow::MessageReceived( msg );
			break;
	}
}

// ---------------------------------------------------------- QuitRequested
bool CPad::QuitRequested( void ) {
	// Lưu tài liệu
	SaveRequested();

	// Báo cho ứng dụng biết cửa sổ đã đóng
	be_app->PostMessage( msg_PadClosed ); // Sử dụng hằng số đã định nghĩa
	return true;
}

// ---------------------------------------------------------- SaveRequested
// ... (Hàm này giữ nguyên) ...
void CPad::SaveRequested( void ) {
	// Mở tệp
	BFile file( mEntry, B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE );
	
	// Thu thập thông tin để đưa vào thuộc tính info
	uint32 info[ 3 ];
	info[ 0 ] = 0x0100;					// Phiên bản 1.0
	info[ 1 ] = mPages.CountItems();	// Số lượng trang
	info[ 2 ] = mCurPage;				// Trang hiện tại
	
	// Duyệt qua các trang, ghi văn bản của trang vào tệp và
	// thêm độ dài của nó vào một mảng sẽ trở thành thuộc tính pages
	uint32 *pages = new uint32[ info[ 1 ] ];
	for( int i = 0; i < info[ 1 ]; i++ ) {
		pages[ i ] = mPages.ItemAt( i )->TextLength();
		file.Write( mPages.ItemAt( i )->Text(), pages[ i ] );
	}
		
	// Cuối cùng, ghi các thuộc tính
	file.WriteAttr( "BEOS:TYPE", B_MIME_TYPE, 0, kPadMimeType,
			strlen( kPadMimeType ) );
	file.WriteAttr( kInfoAttr, B_RAW_TYPE, 0, &info, sizeof( info ) );
	file.WriteAttr( kPagesAttr, B_RAW_TYPE, 0, pages,
			sizeof( *pages ) * info[ 1 ] );
}


// --------------------------------------------------------------- NextPage
// ... (Các hàm còn lại từ CPad.cpp giữ nguyên) ...
// ... (NextPage, PrevPage, NewPage, DelPage, GoToPage, GetDefaultPad, NewPad, ReadPad100, SetPageNum, SetupWindow) ...
// ... (Bạn không cần thay đổi gì thêm trong CPad.cpp) ...
void CPad::NextPage( void ) {
	// Nếu trang tiếp theo đã tồn tại, hãy chuyển đến nó
	if( mCurPage < mPages.CountItems() ) {
		GoToPage( mCurPage + 1 );

	// Nếu không, hãy hỏi để tạo nó
	} else {
		BAlert *alert = new BAlert( NULL,
				"This is the last page.\nCreate a new one?", "No", "Yes",
				NULL, B_WIDTH_AS_USUAL, B_IDEA_ALERT );
		alert->SetShortcut( 2, B_ESCAPE );
		int32 result = alert->Go();
		
		// Người dùng muốn có một cửa sổ mới
		if( result == 1 ) {
			NewPage();
		}
	}
}

// --------------------------------------------------------------- PrevPage
void CPad::PrevPage( void ) {
	// Nếu đây không phải là trang đầu tiên, hãy chuyển đến trang trước đó
	if( mCurPage > 1 ) {
		GoToPage( mCurPage - 1 );
	
	// Nếu không, hãy cho người dùng biết
	} else {
		BAlert *alert = new BAlert( NULL, "This is the first page!",
				"OK", NULL, NULL, B_WIDTH_AS_USUAL, B_IDEA_ALERT );
		alert->Go();
	}
}

// ---------------------------------------------------------------- NewPage
void CPad::NewPage( void ) {
	// Tạo trang mới và thêm nó vào danh sách trang
	BRect bounds( 0, mMenu->Frame().bottom + 1,
			mScroll->Frame().left - 1, mFooter->Frame().top - 2 );
	mPages.AddItem( new CText( bounds, "Text", B_FOLLOW_ALL_SIDES,
			B_WILL_DRAW | B_NAVIGABLE ),mCurPage );

	// Đi đến trang mới
	NextPage();
}

// ---------------------------------------------------------------- DelPage
void CPad::DelPage( void ) {
	// Đảm bảo người dùng muốn làm điều này
	BAlert *alert = new BAlert( NULL,
			"Deleting a note is not undoable.\nDo you really want to do this?",
			"Cancel", "OK", NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT );
	alert->SetShortcut( 1, B_ENTER );
	alert->SetShortcut( 0, B_ESCAPE );
	int32 result = alert->Go();

	// Nếu họ thực sự muốn xóa trang
	if( result == 1 ) {
	
		uint16 page = mCurPage;
		// Nếu đây là trang đầu tiên...
		if( mCurPage == 1 ) {
			// Nếu nó cũng là trang duy nhất, hãy tạo một trang mới
			if( mPages.CountItems() == 1 ) {
				NewPage();
			
			// Nếu không, chỉ cần chuyển sang trang tiếp theo
			} else {
				NextPage();
			}
			mCurPage--;
		
		// Nếu đó là trang cuối cùng (và không phải là trang đầu tiên)...
		} else {
			PrevPage();
		}
		
		// Hủy trang và đặt số trang
		delete mPages.RemoveItemAt( page - 1 );
		SetPageNum();
	}
}

// --------------------------------------------------------------- GoToPage
void CPad::GoToPage( uint16 num ) {
	// Nếu hiện có một trang đang được hiển thị, hãy xóa nó
	BRect frame;
	CText *page;
	if( mCurPage > 0 ) {
		page = mPages.ItemAt( mCurPage - 1 );
		page->Select( 0, 0 );
		page->ScrollToSelection();
		frame = page->Frame();
		RemoveChild( page );
	} else {
		frame.Set( 0, mMenu->Frame().bottom + 1, mScroll->Frame().left - 1,
				mFooter->Frame().top - 3 );
	}
		
	// Thêm trang được yêu cầu
	page = mPages.ItemAt( num - 1 );
	mScroll->SetTarget( page );
	AddChild( page );
	page->ResizeTo( frame.right - frame.left, frame.bottom - frame.top );
	page->MakeFocus();
	mCurPage = num;
	SetPageNum();
}

// ---------------------------------------------------------- GetDefaultPad
BEntry *CPad::GetDefaultPad( bool /*create*/ ) {
	// Hiện tại, chỉ cần đặt pad mặc định trong /boot/system/settings
	return new BEntry( "/boot/system/settings/Pad_default" );
}

// ----------------------------------------------------------------- NewPad
void CPad::NewPad( void ) {
	// Tạo một trang duy nhất và thêm nó vào danh sách
	BRect bounds( 0, mMenu->Frame().bottom + 1, mScroll->Frame().left - 1,
			mFooter->Frame().top - 2 );
	CText *page = new CText( bounds, "Text", B_FOLLOW_ALL_SIDES,
			B_WILL_DRAW | B_NAVIGABLE );
	mPages.AddItem( page, 0 );
	
	// Đi đến trang
	mCurPage = 1;
	GoToPage( mCurPage );
}

// ------------------------------------------------------------- ReadPad100
bool CPad::ReadPad100( BFile &file ) {
	bool success = true;

	// Đọc số lượng trang và trang hiện tại
	uint32 info[ 3 ];
	if( file.ReadAttr( kInfoAttr, B_RAW_TYPE, 0, &info, sizeof( info ) )
			== sizeof( info ) ) {
		
		// Lấy mảng kích thước các trang
		uint32 *pages = new uint32[ info[ 1 ] ];
		if( file.ReadAttr( kPagesAttr, B_RAW_TYPE, 0, pages, info[ 1 ] *
				sizeof( *pages ) ) == info[ 1 ] * sizeof( *pages ) ) {
			
			// Tạo các trang
			char *text = NULL;
			for( int i = 0; i < info[ 1 ]; i++ ) {
				// Đọc văn bản trang
				text = new char[ pages[ i ] ];
				file.Read( text, pages[ i ] );

				// Tạo trang mới và thêm nó vào danh sách trang
				BRect bounds( 0, mMenu->Frame().bottom + 1,
						mScroll->Frame().left - 1, mFooter->Frame().top - 2 );
				mPages.AddItem( new CText( bounds, "Text", B_FOLLOW_ALL_SIDES,
						B_WILL_DRAW | B_NAVIGABLE ), i);

				// Đặt văn bản của trang
				mPages.ItemAt( i )->SetText( text, pages[ i ] );
				//delete [] text;
			}
			
			// Đi đến trang hiện tại
			GoToPage( info[ 2 ] );
		
		// Không đọc được thuộc tính pages
		} else {
			success = false;
		}
	// Không đọc được thuộc tính info
	} else {
		success = false;
	}
	return success;
}

// ------------------------------------------------------------- SetPageNum
void CPad::SetPageNum( void ) {
	char pagestr[ 31 ];
	sprintf( pagestr, "page %d of %d", mCurPage, mPages.CountItems() );
	mFooter->SetText( pagestr );
}

// ------------------------------------------------------------ SetupWindow
void CPad::SetupWindow( void ) {
	BRect bounds = Bounds();

	// Tạo thanh menu
	mMenu = new BMenuBar( BRect( bounds.left, bounds.top, bounds.right,
			bounds.top + 2 ), "MenuBar" );

	// Menu Tệp
	// Menu Tệp
	BMenu *menu = new BMenu( "File" );
	
	// --- Bắt đầu phần thay đổi ---
	
	// 1. Mục "Open..."
	BMenuItem *item = new BMenuItem( "Open...", new BMessage( msg_Open ), 'O' ); // Thêm shortcut Ctrl+O
	menu->AddItem( item );
	item->SetTarget( this ); // Gửi message đến cửa sổ này (CPad)
	
	// 2. Mục "Save"
	item = new BMenuItem( "Save", new BMessage( msg_Save ), 'S' ); // Thêm shortcut Ctrl+S
	menu->AddItem( item );
	item->SetTarget( this ); // Gửi message đến cửa sổ này (CPad)
	
	menu->AddSeparatorItem(); // Thêm một đường phân cách
	
	item = new BMenuItem( "About Pad...",
			new BMessage( B_ABOUT_REQUESTED ) );
	item->SetTarget( be_app );
	menu->AddItem( item );

	item = new BMenuItem( "Quit", new BMessage( B_QUIT_REQUESTED ), 'Q' ); // Thêm shortcut Ctrl+Q
	item->SetTarget( be_app );
	menu->AddItem( item );
	
	mMenu->AddItem( menu );
	
	// Menu Chỉnh sửa
	menu = new BMenu( "Edit" );
	item = new BMenuItem( "Cut", new BMessage( B_CUT ), 'X' );
	menu->AddItem( item );
	item->SetTarget( NULL, this );
	
	item = new BMenuItem( "Copy", new BMessage( B_COPY ), 'C' );
	menu->AddItem( item );
	item->SetTarget( NULL, this );
	
	item = new BMenuItem( "Paste", new BMessage( B_PASTE ), 'V' );
	menu->AddItem( item );
	item->SetTarget( NULL, this );
	
	mMenu->AddItem( menu );
	
	// Menu Ghi chú
	menu = new BMenu( "Note" );
	item = new BMenuItem( "Next note", new BMessage( msg_Next ) );
	menu->AddItem( item );
	
	item = new BMenuItem( "Previous note", new BMessage( msg_Prev ) );
	menu->AddItem( item );
	
	menu->AddSeparatorItem();
	
	item = new BMenuItem( "New note", new BMessage( msg_New ) );
	menu->AddItem( item );
	
	item = new BMenuItem( "Delete note", new BMessage( msg_Del ) );
	menu->AddItem( item );
	
	mMenu->AddItem( menu );
	
	// Thêm thanh menu
	AddChild( mMenu );
	BRect menuframe = mMenu->Frame();
		
	// Tạo các nút trang
	float width, height;
	BButton *button1 = new BButton( BRect( 0, bounds.bottom - 2,
			2, bounds.bottom ), "Prev", "Prev", new BMessage( msg_Prev ),
			B_FOLLOW_LEFT | B_FOLLOW_BOTTOM, B_WILL_DRAW );
	AddChild( button1 );
	button1->GetPreferredSize( &width, &height );
	button1->MoveTo( 0, bounds.bottom - height );
	button1->ResizeTo( width, height );
	BRect b1frame = button1->Frame();

	BButton *button2 = new BButton( BRect( bounds.right - 2,
			bounds.bottom - 2, bounds.right, bounds.bottom ), "Next",
			"Next", new BMessage( msg_Next ), B_FOLLOW_RIGHT |
			B_FOLLOW_BOTTOM, B_WILL_DRAW );
	AddChild( button2 );
	button2->GetPreferredSize( &width, &height );
	button2->MoveTo( bounds.right - width, bounds.bottom - height );
	button2->ResizeTo( width, height );
	BRect b2frame = button2->Frame();
	
	// Tạo chân trang (footer)
	mFooter = new BStringView( BRect( b1frame.right + 1,
			bounds.bottom - height, b2frame.left - 1,
			bounds.bottom ), "Footer", 0, B_FOLLOW_LEFT_RIGHT |
			B_FOLLOW_BOTTOM, B_WILL_DRAW );
	mFooter->SetAlignment( B_ALIGN_CENTER );
	AddChild( mFooter );
	
	// Tạo đường kẻ
	CLine *line = new CLine( BRect( 0, b1frame.top - 1, bounds.right,
			b1frame.top - 1 ), "Line", B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM,
			0, CLine::horiz );
	AddChild( line );
	
	// Tạo thanh cuộn
	mScroll = new BScrollBar( BRect( bounds.right - B_V_SCROLL_BAR_WIDTH,
			menuframe.bottom + 1, bounds.right, b2frame.top - 2 ),
			"ScrollBarV", NULL, 0.0, 0.0, B_VERTICAL );
	AddChild( mScroll );
}
