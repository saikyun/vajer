
(declare concat [[?T] [?T] -> [?T]])
(defn concat
  [l1 l2]
  l1)

(defmacro when
  [args]
  (var pred (in args 0))
  (list 'if pred (concat (list 'do) args)))
