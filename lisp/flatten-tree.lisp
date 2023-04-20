(defn flatten-tree
  [tree]
  (cond
    (nil? tree) []
    (not (list? tree)) [tree]
    :else (reduce concat (map flatten-tree tree))
  )
)