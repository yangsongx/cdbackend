DELETE FROM uc_passport WHERE username='TianFeng' OR username='CSS';
DELETE FROM uc_passport WHERE email='ilovecss@email.com' OR username='ilovecss@email.com';
DELETE FROM uc_passport WHERE username='a_name' OR username='b_name';
DELETE FROM uc_passport WHERE username='abcdefghijklmnopqr';
DELETE FROM uc_passport WHERE email='499528974@qq.com';
DELETE FROM uc_passport WHERE email='active@mail.com';
DELETE FROM uc_passport WHERE usermobile='13022593515' OR usermobile='13022593516' OR usermobile='13022593517';
DELETE FROM uc_passport WHERE usermobile='17705164171' OR usermobile='18911112222';
DELETE FROM uc_passport WHERE usermobile='testinactive' OR usermobile='olderphone';
DELETE FROM uc_session WHERE ticket='aa776f46-cc18-4c44-a7c5-124c7afc45bf' OR ticket='11776f46-cc18-4c44-a7c5-124c7afc45bf' OR ticket='22776f46-cc18-4c44-a7c5-124c7afc45bf';
DELETE FROM uc_session WHERE ticket='33776f46-cc18-4c44-a7c5-124c7afc45bf';
DELETE FROM uc_session;
DELETE FROM uc_sys_sessionconf WHERE sysid=2 OR sysid=99 OR sysid=1;
/* User + Name case */
INSERT INTO uc_passport (username,loginpassword,device,source,createtime,status) VALUES ('TianFeng','123456',0,2,NOW(),1);
INSERT INTO uc_passport (username,loginpassword,device,source,createtime,status) VALUES ('CSS','hellomoto',0,2,NOW(),1);
/* EMAIL case */
INSERT INTO uc_passport (email,loginpassword,device,source,createtime,status) VALUES ('499528974@qq.com','YUANsu1',0,2,NOW(),0);
INSERT INTO uc_passport (email,loginpassword,device,source,createtime,status) VALUES ('active@mail.com','passwd',0,2,NOW(),1);
/* Phone SMS verify case */
INSERT INTO uc_passport (usermobile,accode,codetime) VALUES ('13022593515','123456',UNIX_TIMESTAMP(NOW())+3600);
INSERT INTO uc_passport (usermobile,accode,codetime) VALUES ('13022593516','567890',UNIX_TIMESTAMP(NOW())-10);
INSERT INTO uc_passport (usermobile,accode,codetime) VALUES ('13022593517','123456',UNIX_TIMESTAMP(NOW())+3600);
INSERT INTO uc_passport (usermobile,accode,codetime) VALUES ('olderphone','336688',UNIX_TIMESTAMP(NOW()));
/* Auth case */
INSERT INTO uc_session (caredearid,ticket,session,lastoperatetime) VALUES (100,'aa776f46-cc18-4c44-a7c5-124c7afc45bf',2,NOW());
INSERT INTO uc_session (caredearid,ticket,session,lastoperatetime) VALUES (100,'11776f46-cc18-4c44-a7c5-124c7afc45bf',2,FROM_UNIXTIME(UNIX_TIMESTAMP(NOW())-2593999));
INSERT INTO uc_session (caredearid,ticket,session,lastoperatetime) VALUES (100,'22776f46-cc18-4c44-a7c5-124c7afc45bf',2,FROM_UNIXTIME(UNIX_TIMESTAMP(NOW())-1900));
INSERT INTO uc_sys_sessionconf (sysid,isorder,lefttime,type) VALUES (2,1,2592000,1);
INSERT INTO uc_sys_sessionconf (sysid,isorder,lefttime,type) VALUES (1,1,2592000,1);
INSERT INTO uc_sys_sessionconf (sysid,isorder,lefttime,type) VALUES (99,0,2592000,1);
