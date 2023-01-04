module AVL where
import Prelude hiding (min)

data AVL a = Empty | Node (AVL a) a (AVL a) Int

isEmpty Empty = True
isEmpty _ = False

min::AVL a -> a
min (Node Empty x _ _) = x
min (Node l _ _ _) = min l

insert::(Ord a)=>AVL a -> a -> AVL a
insert t x = fst $ ins t where 
    ins Empty = (Node Empty x Empty 0, 1)
    ins (Node l k r d)
        | x < k     = node (ins l) k (r, 0) d
        | x == k    = (Node l k r d, 0)
        | otherwise = node (l, 0) k (ins r) d


delete::(Ord a)=>AVL a -> a -> AVL a
delete t x = fst $ del t x where 
    del Empty _ = (Empty,0)
    del (Node l k r d) x
        | x<k = node (del l x) k (r,0) d
        | x>k = node (l,0) k (del r x) d
        | isEmpty l = (r,-1)
        | isEmpty r = (l,-1)
        | otherwise = node (l,0) k' (del r k') d where k' = min r

node::(AVL a, Int) -> a -> (AVL a, Int) -> Int -> (AVL a, Int)
node (l,dl) k (r,dr) d = balance (Node l k r d',delta) where 
    d' = d+dr-dl
    delta = deltaH d d' dl dr

deltaH::Int -> Int -> Int -> Int -> Int
deltaH d d' dl dr
    | d>=0&&d'>=0 = dr
    | d<=0&&d'>=0 = d+dr
    | d>=0&&d'<=0 = dl-d
    | otherwise = dl

balance :: (AVL a, Int) -> (AVL a, Int)
--LL
balance (Node (Node (Node a x b dx) y c (-1)) z d (-2), dH) = (Node (Node a x b dx) y (Node c z d 0) 0, dH-1)
--RR
balance (Node a x (Node b y (Node c z d dz)    1)    2, dH) = (Node (Node a x b 0) y (Node c z d dz) 0, dH-1)
--LR
balance (Node (Node a x (Node b y c dy)    1) z d (-2), dH) = (Node (Node a x b dx') y (Node c z d dz') 0, dH-1)
    where dx' = if dy == 1 then -1 else 0
          dz' = if dy == -1 then  1 else 0
--RL
balance (Node a x (Node (Node b y c dy) z d (-1))    2, dH) = (Node (Node a x b dx') y (Node c z d dz') 0, dH-1) 
    where dx' = if dy == 1 then -1 else 0
          dz' = if dy == -1 then  1 else 0
balance (Node (Node a x b dx) y c (-2), dH) = (Node a x (Node b y c (-1)) (dx+1), dH)
balance (Node a x (Node b y c dy)    2, dH) = (Node (Node a x b    1) y c (dy-1), dH)
balance (t, d) = (t, d)

fromList::(Ord a)=>[a] -> AVL a
fromList = foldl insert Empty

toList :: (AVL a) -> [a]
toList Empty = []
toList (Node l k r _) = toList l ++ [k] ++ toList r

instance Show a => Show (AVL a) where
    show Empty = "."
    show (Node l k r d) = "(" ++ show l ++ " " ++
                          show k ++ ":[" ++ show d ++ "] " ++
                          show r ++ ")"
