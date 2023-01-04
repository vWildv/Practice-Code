library(DMwR)
library(car)

data(algae)
plot(algae$NH4)
abline(h=mean(algae$NH4,na.rm=T),lty=1)
abline(h=mean(algae$NH4,na.rm=T)+sd(algae$NH4,na.rm = T),lty=2)
abline(h=median(algae$NH4,na.rm=T),lty=3)

algae[!complete.cases(algae),]
algae1<-na.omit(algae)

###按列数据填补缺失
apply(algae,1,function(x) sum(is.na(x)))
manyNAs(algae,0.2)
algae2<-algae[-manyNAs(algae,0.2),]
algae2[48,'mxPH']<-mean(algae2$mxPH,na.rm = T)
algae2[is.na(algae2$Chla),"Chla"]<-median(algae2$Chla,na.rm=T)

algae3<-algae[-manyNAs(algae,0.2),]
algae3<-centralImputation(algae)

cor(algae[,4:18],use='complete.obs')
symnum(cor(algae[,4:18],use='complete.obs'))
algae<-algae[-manyNAs(algae),]
lm(PO4~oPO4,data=algae)
fillPO4<-function(oP){
  if(is.na(oP)) return(NA)
  else return(42.97+1.293*oP)
}
algae[is.na(algae$PO4),'PO4']<-sapply(
  algae[is.na(algae$PO4),'oPO4'],fillPO4)

###按行数据填补缺失
data(algae)
algae<-algae[-manyNAs(algae),]
clean.algae<-knnImputation(algae,k=10)

###线性回归
lm.a1<-lm(a1~.,data=clean.algae[,1:12])
summary(lm.a1)

#最优子集
library(leaps)
reg.full<-regsubsets(a1~.,data=clean.algae[,1:12])
reg.summary<-summary(reg.full)
coef(reg.full,id=which.min(reg.summary$bic))
#向后消元
anova(lm.a1)
lm2.a1<-update(lm.a1,.~.-season)
summary(lm2.a1)
anova(lm.a1,lm2.a1)

final.lm<-step(lm.a1)
summary(final.lm)

###决策树
library(rpart)
library(rpart.plot)
data(algae)
algae<-algae[-manyNAs(algae),]
rt.a1<-rpart(a1~.,data=algae[,1:12])
prp(rt.a1)

plotcp(rt.a1)
printcp(rt.a1)
rt.a1<-rpartXse(a1~.,data=algae[,1:12])
prp(rt.a1)

first.tree<-rpart(a1~.,data=algae[,1:12])
tree.a1<-snip.rpart(first.tree,c(4,7))
prp(tree.a1)

###svm
library(e1071)
svm.a1<-svm(a1~.,clean.algae[,1:12])
svm.p.a1<-predict(svm.a1,clean.algae)

###提升法
library(gbm)
gbm.a1<-gbm(a1~.,clean.algae[,1:12],distribution = "gaussian",n.trees = 5000,shrinkage = 0.001)
gbm.p.a1<-predict(gbm.a1,clean.algae,n.trees = 5000)
#回归损失函数使用高斯，分类使用伯努利

###模型评估
lm.p.a1<-predict(final.lm,clean.algae)
rt.p.a1<-predict(rt.a1,algae)
mae.a1.lm<-mean(abs(lm.p.a1-algae[,'a1']))
mae.a1.rt<-mean(abs(rt.p.a1-algae[,'a1']))
mae.a1.svm<-mean(abs(svm.p.a1-algae[,'a1']))
mae.a1.gbm<-mean(abs(gbm.p.a1-algae[,'a1']))

nmse.a1.lm<-mean((lm.p.a1-algae[,'a1'])^2)/
  mean((mean(algae[,'a1'])-algae[,'a1'])^2)
nmse.a1.rt<-mean((rt.p.a1-algae[,'a1'])^2)/
  mean((mean(algae[,'a1'])-algae[,'a1'])^2)
nmse.a1.svm<-mean((svm.p.a1-algae[,'a1'])^2)/
  mean((mean(algae[,'a1'])-algae[,'a1'])^2)
nmse.a1.gbm<-mean((gbm.p.a1-algae[,'a1'])^2)/
  mean((mean(algae[,'a1'])-algae[,'a1'])^2)

regr.eval(algae[,'a1'],rt.p.a1,train.y = algae[,'a1'])

sen.lm.p.a1<-ifelse(lm.p.a1<0,0,lm.p.a1)
regr.eval(algae[,'a1'],lm.p.a1)
regr.eval(algae[,'a1'],sen.lm.p.a1)

###k折交叉验证，将数据取k-1份，重复n次，取均值nmse
cv.rpart<-function(form,train,test,...){
  m<-rpartXse(form,train,...)
  p<-predict(m,test)
  mse<-mean((p-resp(form,test))^2)
  c(nmse=mse/mean((mean(resp(form,train))-resp(form,test))^2))
}

cv.lm<-function(form,train,test,...){
  m<-lm(form,train,...)
  p<-predict(m,test)
  p<-ifelse(p<0,0,p)
  mse<-mean((p-resp(form,test))^2)
  c(nmse=mse/mean((mean(resp(form,train))-resp(form,test))^2))
}

res<-experimentalComparison(
  c(dataset(a1~.,clean.algae[,1:12],'a1')),
  c(variants('cv.lm'),variants('cv.rpart',se=c(0,0.5,1))),
  cvSettings(3,10,1234)
)

###7种海藻
DSs<-sapply(names(clean.algae)[12:18],function(x,names.attrs){
  f<-as.formula(paste(x,"~."))
  dataset(f,clean.algae[,c(names.attrs,x)],x)
},names(clean.algae)[1:11])

library(randomForest)
cv.rf<-function(form,train,test,...){
  m<-randomForest(form,train,...)
  p<-predict(m,test)
  mse<-mean((p-resp(form,test))^2)
  c(nmse=mse/mean((mean(resp(form,train))-resp(form,test))^2))
}


cv.svm<-function(form,train,test,...){
  m<-svm(form,train,...)
  p<-predict(m,test)
  mse<-mean((p-resp(form,test))^2)
  c(nmse=mse/mean((mean(resp(form,train))-resp(form,test))^2))
}


cv.gbm<-function(form,train,test,...){
  m<-gbm(form,train,distribution="gaussian",shrinkage = 0.001,n.trees =5000)
  p<-predict(m,test,n.trees =5000)
  mse<-mean((p-resp(form,test))^2)
  c(nmse=mse/mean((mean(resp(form,train))-resp(form,test))^2))
}

res.all<-experimentalComparison(DSs,
  c(variants('cv.lm'),
    variants('cv.rpart',se=c(0,0.5,1)),
    variants('cv.rf',ntree=c(200,500,700)),
    variants('cv.svm'),
    variants('cv.gbm')),
  cvSettings(5,10,1234)
)
bestScores(res.all)
compAnalysis(res.all,against = 'cv.svm.v1',datasets = c('a3','a4','a5','a6','a7'))
compAnalysis(res.all,against = 'cv.rf.v2',datasets = c('a1','a2'))


###频率预测
bestModelsNames<-sapply(bestScores(res.all),function(x) x['nmse','system'])
learners <- c(rf='randomForest',rpart='rpartXse',lm='lm',svm='svm')
funcs<-learners[sapply(strsplit(bestModelsNames,'\\.'),function(x) x[2])]
parSetts<-lapply(bestModelsNames, function(x) getVariant(x,res.all)@pars)
bestModels<-list()
for(a in 1:7){
  form<-as.formula(paste(names(clean.algae)[11+a],'~.'))
  bestModels[[a]]<-do.call(funcs[a],c(list(form,clean.algae[,c(1:11,11+a)]),parSetts[[a]]))
}

clean.test.algae<-knnImputation(test.algae,k=10,distData = algae[,1:11])
preds<-matrix(ncol=7,nrow = 140)
for(i in 1:nrow(clean.test.algae)){
  preds[i,]<-sapply(1:7, function(x) predict(bestModels[[x]],clean.test.algae[i,]))
}
avg.preds<-apply(algae[,12:18],2,mean)
apply(((algae.sols-preds)^2),2,mean)/apply((scale(algae.sols,avg.preds,F)^2),2,mean)

