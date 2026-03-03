#include <cstdio>
#include <vector>

#include "matchable/matchable.h"
#include "test_ok.h"



MATCHABLE(Color, Magenta, Cyan, Yellow)
PROPERTYx1_MATCHABLE(Color::Type, color, GlobalSetting, FeatureX)
MATCHABLE_VARIANT_PROPERTY_VALUE(GlobalSetting, FeatureX, color, Color::Yellow::grab())



int main()
{
    test_ok ok;

    // single property (non-nil setting)
    {
        bool calledA = false;
        bool calledB = false;
        bool calledC = false;
        auto fx = GlobalSetting::FeatureX::grab();
        fx.add_color_observer("A", [&](){ printf("single property observed by A\n"); calledA = true; });
        fx.add_color_observer("B", [&](){ printf("single property observed by B\n"); calledB = true; });
        fx.add_color_observer("C", [&](){ printf("single property observed by C\n"); calledC = true; });
        fx.set_color(Color::Magenta::grab());
        TEST_EQ(ok, calledA, true);
        TEST_EQ(ok, calledB, true);
        TEST_EQ(ok, calledC, true);
        calledA = false;
        calledB = false;
        calledC = false;
        fx.del_color_observer("C");
        fx.set_color(Color::Cyan::grab());
        TEST_EQ(ok, calledA, true);
        TEST_EQ(ok, calledB, true);
        TEST_EQ(ok, calledC, false);
        calledA = false;
        calledB = false;
        calledC = false;
        fx.del_color_observer("A");
        fx.set_color(Color::Yellow::grab());
        TEST_EQ(ok, calledA, false);
        TEST_EQ(ok, calledB, true);
        TEST_EQ(ok, calledC, false);
    }

    // single property (nil setting)
    {
        bool calledA = false;
        bool calledB = false;
        bool calledC = false;
        GlobalSetting::Type nil_setting;
        nil_setting.add_color_observer("A", [&](){ printf("single prop for nil: A\n"); calledA = true; });
        nil_setting.add_color_observer("B", [&](){ printf("single prop for nil: B\n"); calledB = true; });
        nil_setting.add_color_observer("C", [&](){ printf("single prop for nil: C\n"); calledC = true; });
        nil_setting.set_color(Color::Magenta::grab());
        TEST_EQ(ok, calledA, true);
        TEST_EQ(ok, calledB, true);
        TEST_EQ(ok, calledC, true);
        calledA = false;
        calledB = false;
        calledC = false;
        nil_setting.del_color_observer("B");
        nil_setting.set_color(Color::Cyan::grab());
        TEST_EQ(ok, calledA, true);
        TEST_EQ(ok, calledB, false);
        TEST_EQ(ok, calledC, true);
    }

    // vector property (non-nil setting)
    {
        bool calledA = false;
        bool calledB = false;
        bool calledC = false;
        auto fx = GlobalSetting::FeatureX::grab();
        fx.add_color_vect_observer("A", [&](){ printf("vector property observed by A\n"); calledA = true; });
        fx.add_color_vect_observer("B", [&](){ printf("vector property observed by B\n"); calledB = true; });
        fx.add_color_vect_observer("C", [&](){ printf("vector property observed by C\n"); calledC = true; });
        fx.set_color_vect({Color::Magenta::grab(), Color::Cyan::grab()});
        TEST_EQ(ok, calledA, true);
        TEST_EQ(ok, calledB, true);
        TEST_EQ(ok, calledC, true);
        calledA = false;
        calledB = false;
        calledC = false;
        fx.del_color_vect_observer("A");
        fx.set_color_vect({Color::Yellow::grab()});
        TEST_EQ(ok, calledA, false);
        TEST_EQ(ok, calledB, true);
        TEST_EQ(ok, calledC, true);
    }

    // vector property (nil setting)
    {
        bool calledA = false;
        bool calledB = false;
        bool calledC = false;
        GlobalSetting::Type nil_setting;
        nil_setting.add_color_vect_observer("A", [&](){ printf("vector prop for nil: A\n");
                                                        calledA = true; });
        nil_setting.add_color_vect_observer("B", [&](){ printf("vector prop for nil: B\n");
                                                        calledB = true; });
        nil_setting.add_color_vect_observer("C", [&](){ printf("vector prop for nil: C\n");
                                                        calledC = true; });
        nil_setting.set_color_vect({Color::Magenta::grab(), Color::Cyan::grab()});
        TEST_EQ(ok, calledA, true);
        TEST_EQ(ok, calledB, true);
        TEST_EQ(ok, calledC, true);
        calledA = false;
        calledB = false;
        calledC = false;
        nil_setting.del_color_vect_observer("C");
        nil_setting.del_color_vect_observer("C"); // double delete should be ok!
        nil_setting.del_color_vect_observer("B");
        nil_setting.add_color_vect_observer("A", [&](){ printf("vector prop for nil: override A to set B!");
                                                        calledB = true; });
        nil_setting.set_color_vect({Color::Yellow::grab()});
        TEST_EQ(ok, calledA, false);
        TEST_EQ(ok, calledB, true);
        TEST_EQ(ok, calledC, false);
    }

    return ok();
}
