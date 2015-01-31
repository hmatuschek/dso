#
# Integrator Simulation 
#

# Data
X <- as.matrix(read.csv("DSO-REC-2015-01-29-1.csv", sep="\t"))
#X <- as.matrix(read.csv("Uranerz_2014-10-01-1.csv.bz2", sep="\t"))

# Samples per event
N <- ncol(X);
# Sample rate
Fs <- 192e3

# Evaluate filter kernel
R <- 0.5e3; C <- 10e-9; lam <- 1/(2*pi*R*C*Fs)
print(sprintf("Kernel length (sample): %s", 2./lam))
kernel = rep(0, N); 
for (i in 1:N) { kernel[i] <- exp(-(lam*(N-i)))*lam; }

# Filter
D <- matrix(0, nrow=nrow(X), ncol=N);
for (i in 1:nrow(X)) {
  D[i,] <- convolve(X[i,], kernel, type="open")[1:N];
}

# plot
par(mfrow=c(3,1), mar=c(3,2,2,1))
matplot(t(X), type="l", col=1:1, lty=3:3, main="detections")
matplot(t(D), type="l", col=1:1, lty=3:3, main="filtered")

m_X <- c(); m_D <- c();
for (r in 1:nrow(D)) { m_X <- c(m_X, -min(X[r,])); m_D <- c(m_D, -min(D[r,])); }
dens_X <- density(m_X, from=0, kernel="gaussian", bw="ucv")
dens_D <- density(m_D, from=0, kernel="gaussian", bw="ucv")
plot(dens_X, main=sprintf("density estimate (N=%s)", nrow(D)))
lines(dens_D, lty=2)
rug(res)




