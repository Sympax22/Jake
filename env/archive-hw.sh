name=$1 # buddy
number=$2 # 3
both=$name.$number
git archive --prefix=$both/ hw/$name -o $both.tar.gz
