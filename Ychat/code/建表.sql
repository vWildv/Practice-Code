create database pythondb

CREATE TABLE `friend` (
  `uid1` char(20) NOT NULL,
  `uid2` char(20) NOT NULL,
  PRIMARY KEY (`uid1`,`uid2`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8

CREATE TABLE `user` (
  `uid` char(20) NOT NULL,
  `upasswd` char(255) NOT NULL,
  `port` int(11) NOT NULL,
  PRIMARY KEY (`uid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8

INSERT INTO `user` VALUES ('lzhsb', '1213', 1219);
INSERT INTO `user` VALUES ('wdpnb', '1213', 1222);
INSERT INTO `user` VALUES ('wei104718', '1213', 1213);
INSERT INTO `user` VALUES ('yc0620', '1213', 1216);

INSERT INTO `friend` VALUES ('wei104718', 'yc0620');
