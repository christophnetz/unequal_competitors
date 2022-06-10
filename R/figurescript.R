library(reshape2)
library(ggplot2)
library(scales)
library(readr)
library(tidyverse)
library(ggpubr)


###
#Figure 1

data <- read.table("../ifd_finder_grid/results/run1_ifdopt.txt", sep="\t", header = T)


p_spat <- ggplot(data, aes(resource, color = as.factor(comp)))+
  stat_density(   geom="line",position="identity", size=1) +
  theme_bw()+theme(axis.text.x= element_text(size = 6))+labs(x="resource abundance", y="density")+
  labs(color = "Competitive ability")+ theme(legend.position = "bottom") + 
  guides( color = guide_legend( nrow = 1), override.aes = list(size = 10) )



ggsave("fig1_assortment.png", p_spat, width = 6.5, height = 4)

######
#Figure 2


data <- read.table("../ifd_evol/results/nochange_inilowcomp.txt", sep="\t", header = F)

df <- (t(subset(data, select = -c(V1, V1002))))
colnames(df) <- data$V1

minwv = 0.0;     # minimal (weight) value
maxwv = 1.5;     # maximal (weight) value
steps = 100;     # num. of bins across range
stepsize = (maxwv - minwv)/steps  # bin range size


#Frequency matrix:
mtrxwP1 <- matrix(nrow = steps, ncol = length(df[1,]), dimnames=list(seq(minwv+stepsize, maxwv, stepsize)))  # Frequency matrix

for (i in 1:length(df[1,])){
  
  mtrxwP1[,i] = table(cut(df[,i], seq(minwv, maxwv, stepsize), right=T))/1000
  
}

colnames(mtrxwP1) <- data$V1

#load other matrix
data <- read.table("../ifd_evol/results/nochange_inihighcomp.txt", sep="\t", header = F)


df <- (t(subset(data, select = -c(V1, V1002))))
colnames(df) <- data$V1

minwv = 0.0;     # minimal (weight) value
maxwv = 1.5;     # maximal (weight) value
steps = 100;     # num. of bins across range
stepsize = (maxwv - minwv)/steps  # bin range size


#Frequency matrix:
mtrxwP2 <- matrix(nrow = steps, ncol = length(df[1,]), dimnames=list(seq(minwv+stepsize, maxwv, stepsize)))  # Frequency matrix

for (i in 1:length(df[1,])){
  
  mtrxwP2[,i] = table(cut(df[,i], seq(minwv, maxwv, stepsize), right=T))/1000
  
}

colnames(mtrxwP2) <- data$V1

#Overlay both matrices
mtrxwP2[which(mtrxwP2 - mtrxwP1 < 0)] = 0
mtrxwP1[which(mtrxwP1 - mtrxwP2 < 0)] = 0
mtrxwP1 = mtrxwP1+mtrxwP2


P_comp <- ggplot(data = melt(t(mtrxwP1)), aes(x=Var1, y=Var2, fill=value)) + labs(x="generations", y="competitive ability") + 
  geom_tile() + scale_fill_gradientn(colours = colorRampPalette(c("white", "red", "blue"))(3), 
                                     values = c(0, 0.05 , 1), space = "Lab", guide = FALSE) + geom_hline(yintercept = 0)+ theme_bw() +
  theme(axis.title.x=element_text(size=12), axis.title.y=element_text(size=12), panel.grid.major = element_blank(), panel.grid.minor = element_blank(),axis.line = element_line(colour = "black"), legend.position = "none")+
  scale_y_continuous(labels = scales::number_format(accuracy = 0.01)) +geom_hline(yintercept = 1.0)+xlim(0, 5000)


ggsave("fig2_nochange.png", P_comp, width = 15, height = 5.5, units = "cm")



####
#Figure 3

data <- read.table( "../ifd_evol/results/run1comp.txt", sep="\t", header = F)

df <- (t(subset(data, select = -c(V1, V1002))))
colnames(df) <- data$V1

length(df[1,])

minwv = 0.0;     # minimal (weight) value
maxwv = 1.5;     # maximal (weight) value
steps = 100;     # num. of bins across range
stepsize = (maxwv - minwv)/steps  # bin range size


#Frequency matrix:
mtrxwP1 <- matrix(nrow = steps, ncol = length(df[1,]), dimnames=list(seq(minwv+stepsize, maxwv, stepsize)))  # Frequency matrix

for (i in 1:length(df[1,])){
  
  mtrxwP1[,i] = table(cut(df[,i], seq(minwv, maxwv, stepsize), right=T))/1000
  
}

colnames(mtrxwP1) <- data$V1


P_comp <- ggplot(data = melt(t(mtrxwP1)), aes(x=Var1, y=Var2, fill=value)) + labs(x="generations", y="competitive ability") + 
  geom_tile() + scale_fill_gradientn(colours = colorRampPalette(c("white", "red", "blue"))(3), 
                                     values = c(0, 0.05 , 1), space = "Lab", guide = FALSE) + geom_hline(yintercept = 0)+ theme_bw() +
  theme(axis.title.x=element_text(size=12), axis.title.y=element_text(size=12), panel.grid.major = element_blank(), panel.grid.minor = element_blank(),axis.line = element_line(colour = "black"), legend.position = "none")+
  scale_y_continuous(labels = scales::number_format(accuracy = 0.01)) 

ggsave("fig3_branching.png", P_comp, width = 15, height = 5.5, units = "cm")


###
#Figure 4
data <- read.table( "../ifd_evol/results/run1_ifd.txt", sep="\t", header = TRUE)

# categorize morphs, boundaries to be adjusted as appropriate
data <- mutate(data, morph = cut(comp, c(0.0, 0.45, 0.58, 0.7, 1.0, 1.25, 2), labels = paste0("morph", 1:6)))


data <- data %>% left_join(
  data %>%
    group_by(morph) %>%
    summarize(
      meancomp = round(mean(comp), 2),
    ), by="morph"
)


data <- data %>%
  group_by(i, meancomp, IFD) %>%
  summarize(
    meanintake = mean(intake)
  )

data$IFD <- factor(data$IFD , levels=c("1", "0"))

p1 <-  ggplot(data, aes(x = as.factor(IFD), y = meanintake - meancomp  * 0.005 , colour = as.factor(meancomp)))+
  geom_boxplot( )+labs(x="", y = "average net intake") + 
  guides(colour=guide_legend(title="competitive ability"))+theme_bw()+
  scale_x_discrete(labels=c("1" = "At IFD", "0" = "After change"))+ylim(NA, 0.003)


data <- read_delim("run1_landscape.txt", delim="\t")
data <- read.table( "../ifd_evol/results/run1_landscape.txt", sep="\t", header = TRUE)

data <- mutate(data, morph = cut(comp, c(0.0, 0.45, 0.58, 0.7, 1.0, 1.25, 2), labels = paste0("morph", 1:6)))


data <- data %>% left_join(
  data %>%
    group_by(morph) %>%
    summarize(
      meancomp = round(mean(comp), 2),
    ), by="morph"
)


#final fitness values (food corrected for costs)
p2 <- ggplot(filter(data, time == max(time)), aes(as.factor(meancomp), food - comp * 0.005 * 100, color = as.factor(meancomp)))+
  geom_violin(show.legend = FALSE)+ylab("total net intake")+xlab("competitive ability")+theme_bw()

fig4 <- ggarrange(p1, p2, labels = c("A", "B"), ncol = 1, common.legend = TRUE, legend= "right")

ggsave("fig4_intake.png", fig4, width = 6.5, height = 4.5)

### 
#Supplement
#Figure S1
#Houston & McNamara

# 180 inds, 90 with 50% comp of the other 90
#Patch B with 50 % resources of patch A

#IFD 1: 0 : 90
#       2 : 89
#       4 : 88


N = 180

a1 <- seq(N/2, N/4, -1)
b1 <- seq(0, N/2, 2)

dist1 <- factorial(N/2)/(factorial(a1)*factorial(N/2-a1)) * factorial(N/2)/(factorial(b1)*factorial(N/2-b1))

df <- data.frame((a1+b1)/N, dist1/sum(dist1), "HM 88")
names(df) <- c("ratio", "n", "scenario")

#Simulations

data <- read.table( "../ifd_finder_fewpatches/results/HMcomparison_ifdopt2patch.txt", sep="\t", header = TRUE)

#Create cell and individual ID
data <- data %>% 
  mutate(ratio = numpresence/(data$numpresence[1]+data$numpresence[2]))


df2 <- filter(data, resources == 2) %>% count(ratio)
df2$n <- df2$n/sum(df2$n)
df2$scenario <- "internal"
df <- rbind(df, df2)


data <- read.table( "../ifd_finder_fewpatches/results/HMcomparison_ifdopt2patchoutside.txt", sep="\t", header = TRUE)

#Create cell and individual ID
data <- data %>% 
  mutate(ratio = numpresence/(data$numpresence[1]+data$numpresence[2]))


df2 <- filter(data, resources == 2) %>% count(ratio)
df2$n <- df2$n/sum(df2$n)
df2$scenario <- "external"

df <- rbind(df, df2)

df$scenario <- factor(df$scenario , levels = c("HM 88", "external", "internal"))

S1_plot <- ggplot(df)+
  geom_line(aes(x=ratio, y=n, colour=scenario))+
  geom_vline(xintercept = 0.667) + theme_bw()+ylab("Probability of distribution")+xlab("Proportion of all animals on location A")+
  scale_color_manual(values = c("#00BA38","#F8766D",  "#619CFF"))
 
ggsave("S1_IFDs.png", S1_plot, width = 6.5)

#Figure S2


data <- read.table( "../ifd_finder_fewpatches/results/p2t2_ifdopt2patch.txt", sep="\t", header = TRUE)

#Create cell and individual ID
data <- data %>% 
  mutate(ratio = numpresence/(2000  ),
         patches = 2,
         types = 2
  )

df <- select(data.frame(data), resources, ratio, patches, types)

data <- read.table( "../ifd_finder_fewpatches/results/p2t5_ifdopt2patch.txt", sep="\t", header = TRUE)

#Create cell and individual ID
data <- data %>% 
  mutate(ratio = numpresence/(2000  ),
         patches = 2,
         types = 5
  )

df <- rbind(df, select(data.frame(data), resources, ratio, patches, types))


data <- read.table( "../ifd_finder_fewpatches/results/p10t2_ifdopt2patch.txt", sep="\t", header = TRUE)

#Create cell and individual ID
data <- data %>% 
  mutate(ratio = numpresence/(2000  )
  )

#For p10 sims
data$run <- rep(1:1000, each = 10)
data <- data %>% group_by(sim, run, resources)%>% summarize(ratio = sum(ratio))
data$patches <- 10
data$types <- 2

df <- rbind(df, select(data.frame(data), resources, ratio, patches, types))

data <- read.table( "../ifd_finder_fewpatches/results/p10t5_ifdopt2patch.txt", sep="\t", header = TRUE)

#Create cell and individual ID
data <- data %>% 
  mutate(ratio = numpresence/(2000  )
  )

#For p10 sims
data$run <- rep(1:1000, each = 10)
data <- data %>% group_by(sim, run, resources)%>% summarize(ratio = sum(ratio))
data$patches <- 10
data$types <- 5

df <- rbind(df, select(data.frame(data), resources, ratio, patches, types))

df$sim <- "random"
df1 <- df

######


data <- read.table( "../ifd_finder_fewpatches/results/p2t2_ifdopt2patchoutside.txt", sep="\t", header = TRUE)

#Create cell and individual ID
data <- data %>% 
  mutate(ratio = numpresence/(2000  ),
         patches = 2,
         types = 2
  )

df <- select(data.frame(data), resources, ratio, patches, types)

data <- read.table( "../ifd_finder_fewpatches/results/p2t5_ifdopt2patchoutside.txt", sep="\t", header = TRUE)

#Create cell and individual ID
data <- data %>% 
  mutate(ratio = numpresence/(2000  ),
         patches = 2,
         types = 5
  )

df <- rbind(df, select(data.frame(data), resources, ratio, patches, types))


data <- read.table( "../ifd_finder_fewpatches/results/p10t2_ifdopt2patchoutside.txt", sep="\t", header = TRUE)

#Create cell and individual ID
data <- data %>% 
  mutate(ratio = numpresence/(2000  )
  )

#For p10 sims
data$run <- rep(1:1000, each = 10)
data <- data %>% group_by(sim, run, resources)%>% summarize(ratio = sum(ratio))
data$patches <- 10
data$types <- 2

df <- rbind(df, select(data.frame(data), resources, ratio, patches, types))

data <- read.table( "../ifd_finder_fewpatches/results/p10t5_ifdopt2patchoutside.txt", sep="\t", header = TRUE)

#Create cell and individual ID
data <- data %>% 
  mutate(ratio = numpresence/(2000  )
  )

#For p10 sims
data$run <- rep(1:1000, each = 10)
data <- data %>% group_by(sim, run, resources)%>% summarize(ratio = sum(ratio))
data$patches <- 10
data$types <- 5

df <- rbind(df, select(data.frame(data), resources, ratio, patches, types))

df$sim <- "extern"
df2 <- rbind(df1, df)

fig_S2 <- ggplot(filter(df2, resources==2), aes(ratio, colour = sim))+
  geom_density()+facet_grid(patches~types, labeller = label_both)+geom_vline(xintercept = 2/3)+
  theme_bw()+xlab("Proportion of all animals on patches A")+ylab("Density")+
  scale_color_manual(values=c("#F8766D", "#619CFF"), labels = c("external", "internal"))+ guides(color=guide_legend(title="scenario"))

ggsave("S2_patches.png", fig_S2,  width = 6.5, height = 3)
