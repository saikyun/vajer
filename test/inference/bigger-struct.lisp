(declare printf [[:char] ?T -> :void])
(defstruct Cat {name [:char] age :int})

(defn print_cat
  [cat]
  (var name (get cat name))
  (var age (get cat age))
  (printf "name: %s\n" name)
  (printf "age: %d\n" age))

(defn main []
    (var cat (cast :Cat {name "Rea" age 5}))
    0)