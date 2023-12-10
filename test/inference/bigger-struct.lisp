(defstruct Cat {name [:char] age :int})
(defstruct Dog {name [:char] age :int})

(declare print_cat [:Cat -> :void])
(defn print_cat
  [cat]
  (var namev (get cat name))
  (var agev (get cat age))
  (printf "name: %s\n" namev)
  (printf "age: %d\n" agev))

(declare print_dog [:Dog -> :void])
(defn print_dog
  [cat]
  (var namev (get cat name))
  (var agev (get cat age))
  (printf "name: %s\n" namev)
  (printf "age: %d\n" agev))

(defn main []
  (var cat (cast :Cat {name "Rea" age 5}))
  (var dog (cast :Dog {name "Dog" age 15}))
  (print_cat cat)
  (print_dog dog)
  0)