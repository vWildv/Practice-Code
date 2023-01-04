options(repos=structure(c(CRAN="https://mirrors.tuna.tsinghua.edu.cn/CRAN/"))) 
library(reshape2)
library(dplyr)
library(plotly)
library(ggplot2)
library(caret)
library(corrplot)
library(xts)
library(forecast)

vg<-read.csv("C:\\Work\\R\\final\\data.csv",header=T)
vg$Year_of_Release <- as.numeric(as.character(vg$Year_of_Release))
vg$User_Score <- as.numeric(as.character(vg$User_Score))
vg<-filter(vg,vg$Year_of_Release<=2016)
vg<-arrange(vg,desc(Year_of_Release))
vg[vg==""] <-NA

###描述
year_Count<-vg%>%select(Year_of_Release)%>%group_by(Year_of_Release)%>%summarise(Count=n())
ggplot(year_Count,aes(x=Year_of_Release,y=Count))+geom_line()


sale_NA<-vg%>%group_by(Year_of_Release)%>%summarise(Sum_NA_Sales=sum(NA_Sales))
sale_EU<-vg%>%group_by(Year_of_Release)%>%summarise(Sum_EU_Sales=sum(EU_Sales))
sale_JP<-vg%>%group_by(Year_of_Release)%>%summarise(Sum_JP_Sales=sum(JP_Sales))
sale_Other<-vg%>%group_by(Year_of_Release)%>%summarise(Sum_OH_Sales=sum(Other_Sales))
sum_sales<-data.frame(sale_EU,sale_JP$Sum_JP_Sales,sale_NA$Sum_NA_Sales,sale_Other$Sum_OH_Sales)
colnames(sum_sales)<-c("Year","EU","JP","NS","Other")
ggplot()+geom_line(data=sum_sales,aes(x=Year,y=EU,color="EU"))+
  geom_line(data=sum_sales,aes(x=Year,y=JP,color="JP"))+
  geom_line(data=sum_sales,aes(x=Year,y=NS,color="NA"))+
  geom_line(data=sum_sales,aes(x=Year,y=Other,color="Other"))


vg$company[vg$Platform%in%c("GEN","SCD","DC","GG","SAT")] <- "Sega"
vg$company[vg$Platform%in%c('PC')] <- "PC"
vg$company[vg$Platform%in%c("2600","3DO","NG","PCFX","TG16","WS")] <- "Other"
vg$company[vg$Platform%in%c("3DS","DS","GB","GBA","N64","GC", "NES","SNES","Wii","WiiU")] <- "Nintendo"
vg$company[vg$Platform%in%c("PS","PS2","PSP","PS3","PS4","PSV")] <- "Sony"
vg$company[vg$Platform%in%c("XB","X360", "XOne")] <- "Microsoft"
Platform_level<-vg%>%group_by(Platform,company)%>%summarise(Sales = sum(Global_Sales))
Platform_level<-arrange(Platform_level,company,Sales)
Platform_level$Platform<-factor(Platform_level$Platform,levels = Platform_level$Platform)
ggplot(Platform_level,aes(x="",y=Sales,fill=Platform))+geom_bar(stat = "identity")+
  coord_polar(theta="y")
ggplot(Platform_level,aes(x="",y=Sales,fill=company))+geom_bar(stat = "identity")+
  coord_polar(theta="y")


year_Per<-vg%>%group_by(Platform,company,Year_of_Release)%>%summarise(Sales=sum(Global_Sales))
year_Per<-year_Per%>%group_by(Year_of_Release)%>%mutate(YearTotal=sum(Sales),Percent=Sales/YearTotal)
year_Per$Platform<-factor(year_Per$Platform,levels=Platform_level$Platform)
plot_ly(year_Per,x=~Year_of_Release,y=~Percent,color=~Platform,
        hoverinfo='text',type='bar',
        text=~paste('Year : ', Year_of_Release,'<br>',
                    'Platform : ',Platform,'<br>',
                    'Sales (in millions of units): ', Sales,'<br>',
                    'Percentage in total game sales: ',Percent,
                    'Owner: ',company,'<br>'))%>%layout(barmode='stack')


###预测
vg.data<-vg[,c(6,1,3,5,2,4,11:14)]
na_Per<-apply(vg.data,2, function(x){length(which(is.na(x)))/nrow(vg.data)})
print(na_Per)

vg.data<-vg.data%>%filter(!is.na(Genre)&!is.na(Name))
###bagging 树预测缺失值
data_Pre<-preProcess(vg.data[,-1],method=c("bagImpute"))
### x自变量，y因变量，老的90%做训练集，新的10%做测试集
trainX<-predict(data_Pre,vg.data[vg.data$Year_of_Release<=quantile(vg.data$Year_of_Release,0.9),-1])
testX<-predict(data_Pre,vg.data[vg.data$Year_of_Release>quantile(vg.data$Year_of_Release,0.9),-1])
trainY<-vg.data[nrow(testX)+1:nrow(trainX),1]
testY<-vg.data[1:nrow(testX),1]
vg.data.train<-cbind(trainY,trainX)
###训练集训练时，取更老的90%train,新的10%test
index<-which(vg.data.train$Year_of_Release<quantile(vg.data.train$Year_of_Release,0.9))

###求近3年的开发商平均销量,整合
sale_Pub<-vg.data.train%>%filter(Year_of_Release>=max(Year_of_Release)-3)%>% 
  group_by(Publisher)%>%mutate(avgSales=mean(trainY))%>%ungroup()%>%
  select(Publisher,avgSales)%>%unique()
vg.data.train<-left_join(vg.data.train,sale_Pub,by="Publisher")
vg.data.train[is.na(vg.data.train)]<-0
vg.data.train<-vg.data.train%>%select(-Publisher,-Name)
testX<-left_join(testX,sale_Pub,by="Publisher")
testX[is.na(testX)]<-0
testX<-testX%>%select(-Publisher,-Name)

vg.data.train <- cbind(vg.data.train[,c(1:4)],scale(vg.data.train[,c(5:9)]))
testX <- cbind(testX[,c(1:3)],scale(testX[,c(4:8)]))
cormax <- cor(vg.data.train[,c(5:9)])
corrplot(cormax,method="number")

###km
km<-kmeans(vg.data.train[,-c(2,3,4)],3)
barplot(t(km$centers),beside = T,xlab = "cluster",ylab="value")

###模型
ctrl<-trainControl(method='LGOCV',index=list(TrainSet=index))
set.seed(1213)

f<-as.formula(trainY~Year_of_Release+Genre+Platform+Critic_Score+Critic_Count+User_Score+User_Count)
min_Year<-min(vg.data.train$Year_of_Release)

##lm
lm.NA<-train(f,data=vg.data.train, trControl=ctrl,method="lm",
             weights=exp(vg.data.train$Year_of_Release-min_Year+1))
train_p_lm<-predict(lm.NA,vg.data.train)
test_p_b<-predict(lm.NA,testX)
RMSE(train_p_lm,vg.data.train$trainY)

##svm
svm.NA<-train(f,data=vg.data.train, trControl=ctrl,method="svmRadial",
              weights=exp(vg.data.train$Year_of_Release-min_Year+1))
train_p_svm<-predict(svm.NA,vg.data.train)
test_p_svm<-predict(svm.NA,testX)
RMSE(train_p_svm,vg.data.train$trainY)

##rf
rf.NA<-train(f,data=vg.data.train, trControl=ctrl,method="rf",
             weights=exp(vg.data.train$Year_of_Release-min_Year+1))
train_p_rf<-predict(rf.NA,vg.data.train)
test_p_rf<-predict(rf.NA,testX)
RMSE(train_p_rf,vg.data.train$trainY)

##compare
resamps<-resamples(list(lm=lm.NA,svm=svm.NA,rf=rf.NA))
bwplot(resamps)

##test
train_mean<-mean(vg.data.train$trainY)
RMSE(rep(train_mean,length(testY)), testY) 
RMSE(test_p_b,testY) 
RMSE(test_p_rf,testY) 
RMSE(test_p_svm,testY)
test_stacked<-(test_p_b+test_p_rf+test_p_svm)/3
RMSE(test_stacked,testY)
ggplot(data=NULL,aes(x=(test_p_b+test_p_rf+test_p_svm)/3,y=testY))+geom_point()+geom_abline(slope = 1,intercept = 0)


##time,JP
ts_JP_sale<-xts(sum_sales$JP,as.Date(as.character(sum_sales$Year),format = '%Y'))
plot(ts_JP_sale)

ndiffs(ts_JP_sale)
ts_JP_sale<-diff(ts_JP_sale,lag=1)
plot(ts_JP_sale)

ts_for<-forecast(ts_JP_sale,h=5)
plot(ts_for)

JP.arima<-auto.arima(ts_JP_sale)
JP.pre<-forecast(JP.arima,h=5)
plot(JP.pre)

qqnorm(JP.arima$residuals)
qqline(JP.arima$residuals)
shapiro.test(JP.arima$residuals)
