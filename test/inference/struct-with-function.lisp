(defstruct Dog {name [:char] age :int})

(declare print_dog [:Dog -> :void])
(defn print_dog
  [dog]
  (var namev (get dog name))
  (var agev (get dog age))
  (printf "name: %s\n" namev)
  (printf "age: %d\n" agev))

(defn main []
  (var dog (cast :Dog {name "Dog" age 15}))
  (print_dog dog)
  0)