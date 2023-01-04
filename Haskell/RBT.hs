data RBTree a = Nil | Node Color a (RBTree a) (RBTree a) deriving (show)
data Color = Red | Black deriving (show,Eq)

blacken Nil = Nil
blacken (Node _ value left right) = Node Black value left right

insert x root = blacken $ insert' root where 
    insert' Nil = Node Red x Nil Nil
    insert' root@(Node color y left right)
        | x<y = balance color y (insert' left) right
        | x>y = balance color y left (insert' right)
        | otherwise = root

balance Black z (Node Red x a (Node Red y b c)) d = Node Red y (Node Black x a b) (Node Black z c d)
balance Black z (Node Red y (Node Red x a b) c) d = Node Red y (Node Black x a b) (Node Black z c d)
balance Black x a (Node Red y b (Node Red z c d)) = Node Red y (Node Black x a b) (Node Black z c d)
balance Black x a (Node Red z (Node Red y b c) d) = Node Red y (Node Black x a b) (Node Black z c d)
balance color value left right = Node color value left right

