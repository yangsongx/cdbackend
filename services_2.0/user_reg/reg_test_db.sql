DELETE FROM uc_passport WHERE username='TianFeng' OR username='CSS';
DELETE FROM uc_passport WHERE email='ilovecss@email.com' OR username='ilovecss@email.com';
DELETE FROM uc_passport WHERE username='a_name';
DELETE FROM uc_passport WHERE username='abcdefghijklmnopqr';
DELETE FROM uc_passport WHERE email='499528974@qq.com';
DELETE FROM uc_passport WHERE email='active@mail.com';
DELETE FROM uc_passport WHERE usermobile='13022593515' OR usermobile='13022593516' OR usermobile='13022593517';
DELETE FROM uc_passport WHERE usermobile='17705164171';
/* User + Name case */
INSERT INTO uc_passport (username,loginpassword,device,source,createtime,status) VALUES ('TianFeng','123456',0,2,NOW(),1);
INSERT INTO uc_passport (username,loginpassword,device,source,createtime,status) VALUES ('CSS','hellomoto',0,2,NOW(),1);
/* EMAIL case */
INSERT INTO uc_passport (email,loginpassword,device,source,createtime,status) VALUES ('499528974@qq.com','YUANsu1',0,2,NOW(),0);
INSERT INTO uc_passport (email,loginpassword,device,source,createtime,status) VALUES ('active@mail.com','passwd',0,2,NOW(),1);
/* Phone SMS verify case */
INSERT INTO uc_passport (usermobile,code,codetime) VALUES ('13022593515','123456',UNIX_TIMESTAMP(NOW())+3600);
INSERT INTO uc_passport (usermobile,code,codetime) VALUES ('13022593516','567890',UNIX_TIMESTAMP(NOW())-10);
INSERT INTO uc_passport (usermobile,code,codetime) VALUES ('13022593517','123456',UNIX_TIMESTAMP(NOW())+3600);