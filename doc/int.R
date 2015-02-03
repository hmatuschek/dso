#
# Integrator Simulation 
#

# Data
#X <- as.matrix(read.csv("background.csv", sep="\t"))
X <- as.matrix(read.csv("DSO-REC-2015-02-02-4.csv", sep="\t"))
#X <- as.matrix(read.csv("Uranerz_2014-10-01-1.csv.bz2", sep="\t"))

# Samples per event
N <- ncol(X);
# Sample rate
Fs <- 192e3

m_X <- c();
for (r in 1:nrow(X)) { m_X <- c(m_X, -min(X[r,])); }

# plot
par(mfrow=c(2,1), mar=c(3,2,2,1))
time <- 1e3*seq(0, N-1, length.out=N)/Fs # time in ms.
matplot(time, t(X), type="l", col=1:1, lty=3:3, main="detections")
abline(h=-min(m_X), lty=2)

# Distribution, under the assumption of a simple exponential distr.
a <- min(m_X)
l <- 1./(mean(m_X)-a)
x <- seq(a, max(m_X), length.out=100)
y <- l*exp(-l*(x-a))

dens_X <- density(m_X, from=0, kernel="gaussian", bw="ucv")
plot(dens_X, main=sprintf("peak distribution (N=%s)", nrow(X)))
lines(x,y, lty=2)
rug(m_X, side=1)
#rug(m_D, side=3)




