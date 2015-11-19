def css_properties(css):
    css = css.replace('\n', '').replace('\r', '')

    stage1 = css.split('{')[1:]
    stage2 = [x.split('}')[0] for x in stage1]
    stage3 = [[y.strip() for y in x.split(';') if y.strip()] for x in stage2]
    stage4 = [[y.split(':') for y in x] for x in stage3]
    stage5 = [[tuple([z.strip() for z in y]) for y in x] for x in stage4]

    acc = []
    for each in stage5:
        acc += each

    return acc


if __name__ == '__main__':
    print(css_properties("""
        #LasVegas .billboard { text-decoration: blink; }

        .ninja, #Snowden { visibility: hidden; }


        .oliveoil
        {
          z-index: 1;
        }
        .water
        {
          z-index: 0;
        }

        #poop {
          float  : none  ;
          color  : brown ;

          width  : 15cm  ;
          height : 120cm ;
        }

        .God { position: absolute; display: none; }
        #blackhole { padding: -9999em; }

        .word {  font-family:    "Comic Sans", "Times New Roman", sans-serif  ;  }
        """))
