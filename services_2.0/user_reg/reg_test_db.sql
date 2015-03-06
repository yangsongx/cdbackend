DELETE FROM uc_passport WHERE username='TianFeng' OR username='CSS';
DELETE FROM uc_passport WHERE email='ilovecss@email.com' OR username='ilovecss@email.com';
DELETE FROM uc_passport WHERE username='a_name';
DELETE FROM uc_passport WHERE email='499528974@qq.com';
DELETE FROM uc_passport WHERE email='active@mail.com';
/* User + Name case */
INSERT INTO uc_passport (username,loginpassword,device,source,createtime,status) VALUES ('TianFeng','123456',0,2,NOW(),1);
INSERT INTO uc_passport (username,loginpassword,device,source,createtime,status) VALUES ('CSS','hellomoto',0,2,NOW(),1);
/* EMAIL case */
INSERT INTO uc_passport (email,loginpassword,device,source,createtime,status) VALUES ('499528974@qq.com','YUANsu1',0,2,NOW(),0);
INSERT INTO uc_passport (email,loginpassword,device,source,createtime,status) VALUES ('active@mail.com','passwd',0,2,NOW(),1);