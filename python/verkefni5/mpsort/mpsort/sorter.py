import os.path
import shutil
import stagger


class Sorter(object):
    def __init__(self):
        super().__init__()

    def sort(self, source, dest):
        for root, dirs, files in os.walk(source):
            for name in files:
                file_name = os.path.join(root, name)

                try:
                    tag = stagger.read_tag(file_name)
                    new_name = os.path.join(dest, '{artist}/{album}/{track} - {title}.mp3'.format(
                        artist=(tag.artist or 'Unknown Artist').strip(),
                        album=(tag.album or 'Unknown Almbum').strip(),
                        track=tag.track,
                        title=(tag.title or os.path.basename(file_name)).strip()))

                    os.makedirs(os.path.dirname(new_name), exist_ok=True)
                    shutil.copy2(file_name, new_name)
                except stagger.errors.NoTagError:
                    # ignore files without tags
                    pass
