X <- as.matrix(read.csv("DSO-REC-2015-01-29-1.csv", sep="\t"))
#X <- as.matrix(read.csv("Uranerz_2014-10-01-1.csv.bz2", sep="\t"))
Ns <- ncol(X)
D <- X[,(Ns/2-10):(Ns/2+100)]

par(mfrow=c(2,1), mar=c(3,2,2,1))
matplot(t(D), type="l", col=1:1, lty=3:3, main="detections")

res <- c()
for (r in 1:nrow(D)) { res <- c(res, -min(D[r,])); }
a <- min(res)
l <- 1./(mean(res)-a)
x <- seq(a, max(res), length.out=100)
y <- l*exp(-l*(x-a))
dens <- density(res, from=0, kernel="gaussian", bw="ucv")
plot(dens, main=sprintf("density estimate (N=%s)", nrow(D)))
lines(x,y, lty=2)
rug(res)



