avl_data <- read.table("avl_tries_sorted.txt", header = FALSE)
unbalanced_data <- read.table("unbalanced_tries_sorted.txt", header = FALSE)

names(avl_data) <- "tries"
names(unbalanced_data) <- "tries"

avl_data$index <- 1:nrow(avl_data)
unbalanced_data$index <- 1:nrow(unbalanced_data)

avl_data$type <- "AVL Tree"
unbalanced_data$type <- "Unbalanced Tree"

combined_data <- rbind(avl_data, unbalanced_data)

library(ggplot2)

ggplot(combined_data, aes(x = index, y = tries, color = type)) +
  geom_point(alpha = 0.6, size = 1) +
  labs(title = "Search Tries Scatter Plot: AVL Tree vs Unbalanced Tree",
       subtitle = "Each point represents one search operation - AVL shows consistent performance",
       x = "Search Operation Index",
       y = "Number of Tries",
       color = "Tree Type") +
  scale_color_manual(values = c("AVL Tree" = "blue", "Unbalanced Tree" = "red")) +
  theme_minimal()