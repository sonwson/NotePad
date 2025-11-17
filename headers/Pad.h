#ifndef PAD_H
#define PAD_H

// Định nghĩa các hằng số tin nhắn (Message Constants)
// Các giá trị 'xxxx' là quy ước của BeOS để tạo mã uint32 duy nhất

// Tin nhắn cho Menu Tệp
const uint32 msg_Open = 'mOpn';
const uint32 msg_Save = 'mSav';

// Tin nhắn cho Menu Ghi chú (Note)
const uint32 msg_Next = 'mNxt';
const uint32 msg_Prev = 'mPrv';
const uint32 msg_New = 'mNew';
const uint32 msg_Del = 'mDel';

// Tin nhắn nội bộ của ứng dụng
const uint32 msg_PadClosed = 'mPcl';

// Định nghĩa các hằng số thuộc tính (Attribute Constants)
const char *kPadMimeType = "application/x-jessehall-pad";
const char *kInfoAttr = "Pad:info";
const char *kPagesAttr = "Pad:pages";

#endif // PAD_H
