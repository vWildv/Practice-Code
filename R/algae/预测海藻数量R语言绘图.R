library(ggplot2)
library(DMwR)
library(lattice)

boxplot(algae$oPO4,ylab='Orthophosphate (oPO4)')
rug(jitter(algae$oPO4),side=2)
abline(h=mean(algae$oPO4,na.rm = T),lty=2)

hist(algae$mxPH,probability = T,main='Histogram of max pH value')
lines(density(algae$mxPH,na.rm=T))
rug(jitter(algae$mxPH))
qqPlot(algae$mxPH,main='Normal QQ plot of max pH')

###ggplot
ggplot(algae,aes(x=season,y=a1))+geom_bar(stat = 'identity',aes(fill=size),position = 'dodge')
ggplot(algae,aes(x=oPO4,y=a1))+geom_line()
ggplot(algae,aes(season,fill=size))+geom_histogram(stat = 'count')
ggplot(algae,aes(x=oPO4))+geom_histogram(aes(y=stat(density)))+geom_density(color='red')
ggplot(algae,aes(x=oPO4,color=size,fill=size))+geom_density(alpha=0.4)
ggplot(algae,aes(x=oPO4,color=size,fill=size))+geom_line(stat = 'density')

bw<-function(x,y){
  ggplot(data=algae,aes(x=x,y=y,color=x))+geom_boxplot(aes(color=x),na.rm = T)+
    geom_jitter(shape=16,position=position_jitter(0.2),na.rm = T)+
    stat_summary(fun='mean',geom='point',shape=23,size=3,fill='white',na.rm = T)
    #scale_color_manual(values = c("#999999", "#E69F00", "#56B4E9"))
}

bw(algae$size,algae$Chla)
bw(algae$size,algae$mxPH)

ggplot(data=NULL,aes(x=lm.p.a1,y=algae[,'a1']))+geom_point()+geom_abline(slope = 1,intercept = 0)
ggplot(data=NULL,aes(x=rt.p.a1,y=algae[,'a1']))+geom_point()+geom_abline(slope = 1,intercept = 0)
ggplot(data=NULL,aes(x=svm.p.a1,y=algae[,'a1']))+geom_point()+geom_smooth()+geom_abline(slope = 1,intercept = 0)
ggplot(data=NULL,aes(x=gbm.p.a1,y=algae[,'a1']))+geom_point()+geom_smooth()+geom_abline(slope = 1,intercept = 0)


###lattice
library(lattice)
library(Hmisc)
minO2 <- equal.count(na.omit(algae$mnO2),number=4, overlap=1/5)

histogram(~mxPH|season,data=algae)
histogram(~mxPH|size*speed,data=algae)

stripplot(mxPH~size|speed,data=algae,jitter=T)
stripplot(season ~ a3|minO2,data = algae[!is.na(algae$mnO2),])

bwplot(size~a1,data=algae,ylab='River Size',xlab='Algae a1')
bwplot(size~a1,data = algae,panel=panel.bpplot,probs=seq(.01,.49,by=.01),
       datadensity=T,ylab='River Size',xlab = 'Algae a1')


cloud(a1~PO4*oPO4|size,data=algae,screen=list(z=105,x=-70))
